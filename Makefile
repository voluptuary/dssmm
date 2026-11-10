# Active ezfblue target. Root source/ and images/ are DS Style donors only.
.DEFAULT_GOAL := all
.PHONY: all clean check
all:
	$(MAKE) -C targets/blue
clean:
	$(MAKE) -C targets/blue clean
check:
	python3 tests/test-blue-packaging.py
