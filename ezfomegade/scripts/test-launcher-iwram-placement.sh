#!/usr/bin/env bash
set -euo pipefail

repo_root=$(cd -- "$(dirname -- "$0")/.." && pwd)
target=$(basename "$repo_root")
elf=${1:-"$repo_root/$target.elf"}
devkitarm=${DEVKITARM:-${DEVKITPRO:-/opt/devkitpro}/devkitARM}
nm="$devkitarm/bin/arm-none-eabi-nm"

if [[ ! -x "$nm" || ! -f "$elf" ]]; then
	printf 'Missing devkitARM nm or linked ELF: %s\n' "$elf" >&2
	exit 1
fi

symbol_record()
{
	"$nm" -S --radix=x "$elf" | awk -v name="$1" '$4 == name && !found { print $1, $2; found = 1 }'
}

check_range()
{
	local label=$1
	local record=$2
	local lower=$3
	local upper=$4
	local address size start bytes

	if [[ -z "$record" ]]; then
		printf 'Missing linked symbol: %s\n' "$label" >&2
		exit 1
	fi
	read -r address size <<< "$record"
	start=$((16#$address))
	bytes=$((16#$size))
	if (( start < lower || start + bytes > upper )); then
		printf '%s is outside its expected memory range: 0x%s + 0x%s\n' "$label" "$address" "$size" >&2
		exit 1
	fi
}

for symbol in launcher_scale84_x launcher_scale56_y launcher_scale48_x \
	launcher_scale32_y launcher_scale80_56 launcher_scale80_37 launcher_scale80_32
do
	record=$(symbol_record "$symbol")
	check_range "$symbol" "$record" 0x03000000 0x03008000
done

for symbol in currentpath_temp recents_return_path
do
	record=$(symbol_record "$symbol")
	check_range "$symbol" "$record" 0x02000000 0x02040000
done

data_end=$("$nm" -n "$elf" | awk '$3 == "__data_end__" && !found { print $1; found = 1 }')
irq_sp=$("$nm" -n "$elf" | awk '$3 == "__sp_irq" && !found { print $1; found = 1 }')
if [[ -z "$data_end" || -z "$irq_sp" ]]; then
	printf 'Missing IWRAM data-end or IRQ-stack symbols.\n' >&2
	exit 1
fi
gap=$((16#$irq_sp - 16#$data_end))
if (( gap < 512 )); then
	printf 'IWRAM static-to-IRQ-stack gap is too small: %u bytes\n' "$gap" >&2
	exit 1
fi

symbol_address()
{
	"$nm" -n "$elf" | awk -v name="$1" '$3 == name && !found { print $1; found = 1 }'
}

sbss_end=$(symbol_address __sbss_end__)
heap_start=$(symbol_address __eheap_start)
heap_end=$(symbol_address __eheap_end)
user_sp=$(symbol_address __sp_usr)
if [[ -z "$sbss_end" || -z "$heap_start" || -z "$heap_end" || -z "$user_sp" ]]; then
	printf 'Missing EWRAM static/heap/user-stack linker symbols.\n' >&2
	exit 1
fi

ewram_static_end=$((16#$sbss_end))
ewram_heap_start=$((16#$heap_start))
ewram_heap_end=$((16#$heap_end))
ewram_user_sp=$((16#$user_sp))
ewram_top=$((0x02040000))
user_stack_reserve=$((ewram_user_sp - ewram_heap_end))
if (( ewram_static_end < 0x02000000 || ewram_static_end > ewram_heap_start ||
	ewram_heap_start >= ewram_heap_end || ewram_heap_end >= ewram_user_sp ||
	ewram_user_sp > ewram_top )); then
	printf 'EWRAM static/heap/user-stack ordering is invalid: static end 0x%s, heap 0x%s..0x%s, user SP 0x%s\n' \
		"$sbss_end" "$heap_start" "$heap_end" "$user_sp" >&2
	exit 1
fi
if (( user_stack_reserve < 0x3000 )); then
	printf 'EWRAM user-stack reserve is below the configured 12 KiB minimum: %u bytes\n' \
		"$user_stack_reserve" >&2
	exit 1
fi

printf 'Scale maps are in IWRAM, path buffers are in EWRAM; IWRAM stack gap: %u bytes; EWRAM heap: %u bytes; user-stack reserve: %u bytes (layout guards, not runtime high-water).\n' \
	"$gap" "$((ewram_heap_end - ewram_heap_start))" "$user_stack_reserve"
