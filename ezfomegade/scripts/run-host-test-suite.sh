#!/usr/bin/env bash
set -euo pipefail

script_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
shopt -s nullglob
test_scripts=("$script_dir"/test-*.sh)

if (( ${#test_scripts[@]} == 0 )); then
	printf 'No host test scripts found in %s\n' "$script_dir" >&2
	exit 1
fi

start_time=$SECONDS
for index in "${!test_scripts[@]}"; do
	test_script=${test_scripts[$index]}
	printf '\n[%d/%d, %ds elapsed] %s\n' "$((index + 1))" "${#test_scripts[@]}" \
		"$((SECONDS - start_time))" "${test_script##*/}"
	bash "$test_script"
done

printf '\nAll %d host test scripts passed in %ds.\n' "${#test_scripts[@]}" "$((SECONDS - start_time))"
