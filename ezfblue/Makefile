# Active ezfblue target. Root images are active assets; only targets/blue/source is firmware code.
.DEFAULT_GOAL := all
.PHONY: all clean check
all:
	$(MAKE) -C targets/blue
	python3 scripts/check-blue-linked-layout.py targets/blue/ezbluekernel-development.elf
clean:
	$(MAKE) -C targets/blue clean
check:
	python3 tests/test-blue-save-rts.py
	python3 tests/test-blue-packaging.py
	python3 tests/test-blue-psram-load.py
	bash scripts/test-nes-patch-host.sh
	bash scripts/test-patch-write-host.sh
	python3 tests/test-blue-nor.py
	python3 tests/test-blue-nor-scan.py
	python3 tests/test-blue-nor-metadata.py
	python3 tests/test-blue-flash-guards.py
	python3 tests/test-blue-folder-pool.py
	python3 tests/test-blue-launch-update.py
	python3 tests/test-blue-boot.py
	bash scripts/test-nor-background-host.sh
