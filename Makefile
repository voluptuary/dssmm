.DEFAULT_GOAL := help
.PHONY: help ezfomegade ezfomega ezfblue build-all verify-all check-isolation
.NOTPARALLEL: build-all verify-all
AVAILABLE := $(shell python3 -c 'import json; from pathlib import Path; print(" ".join(p for p in json.load(open("projects.json"))["projects"] if (Path(p)/"project.json").is_file()))')
help:
	@echo "Choose hardware: make ezfomegade | ezfomega | ezfblue"
	@echo "Verify: make verify-ezfomegade | verify-ezfomega | verify-ezfblue"
	@echo "Explicit aggregate: make build-all | verify-all"
ezfomegade ezfomega ezfblue:
	+python3 scripts/project.py build $@
verify-%:
	+python3 scripts/project.py verify $*
build-all: $(AVAILABLE)
verify-all: $(addprefix verify-,$(AVAILABLE))
check-isolation:
	python3 scripts/project_policy.py
