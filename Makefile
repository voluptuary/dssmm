.DEFAULT_GOAL := help
.PHONY: help ezfomegade ezfomega ezfblue build-all verify-all check-isolation
.NOTPARALLEL: build-all verify-all
help:
	@echo "Choose hardware: make ezfomegade | ezfomega | ezfblue"
	@echo "Verify: make verify-ezfomegade | verify-ezfomega | verify-ezfblue"
	@echo "Explicit aggregate: make build-all | verify-all"
ezfomegade ezfomega ezfblue:
	+python3 scripts/project.py build $@
verify-%:
	+python3 scripts/project.py verify $*
build-all: ezfomegade ezfomega ezfblue
verify-all: verify-ezfomegade verify-ezfomega verify-ezfblue
check-isolation:
	python3 scripts/project_policy.py
