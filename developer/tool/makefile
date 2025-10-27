# developer/tool/makefile — Orchestrator (Hybrid)
.SUFFIXES:
.EXPORT_ALL_VARIABLES:

RT_INCOMMON := $(REPO_HOME)/tool_shared/third_party/RT-project-share/release
include $(RT_INCOMMON)/make/environment_RT_1.mk

.PHONY: usage 
usage:
	@printf "Usage: make [usage|information|all|lib|CLI|kmod|clean]\n"

.PHONY: version 
version:
	@printf "local ----------------------------------------\n"
	@echo tool/makefile version 2.0
	@printf "target_library_CLI.mk ----------------------------------------\n"
	@$(MAKE) -f $(RT_INCOMMON)/make/target_kmod.mk version
	@printf "target_kmod.mk ----------------------------------------\n"
	@$(MAKE) -f $(RT_INCOMMON)/make/target_library_CLI.mk version

.PHONY: information
information:
	@printf "local ----------------------------------------\n"
	-@echo CURDIR='$(CURDIR)'
	@echo REPO_HOME="$(REPO_HOME)"
	@echo KMOD_BUILD_DIR="/lib/modules/$(shell uname -r)/build"
	@echo CURDIR="$(CURDIR)"
	@printf "target_library_CLI.mk ----------------------------------------\n"
	@$(MAKE) -f $(RT_INCOMMON)/make/target_library_CLI.mk information
	@printf "target_kmod.mk ----------------------------------------\n"
	@$(MAKE) -f $(RT_INCOMMON)/make/target_kmod.mk information

.PHONY: all
all: library CLI kmod

.PHONY: library lib
library lib:
	@$(MAKE) -f $(RT_INCOMMON)/make/target_library_CLI.mk library

.PHONY: CLI
CLI:
	@$(MAKE) -f $(RT_INCOMMON)/make/target_library_CLI.mk CLI

.PHONY: kmod
kmod:
	@$(MAKE) -f $(RT_INCOMMON)/make/target_kmod.mk kmod

.PHONY: clean
clean:
	@printf "local ----------------------------------------\n"
	@printf "target_library_CLI.mk ----------------------------------------\n"
	@$(MAKE) -f $(RT_INCOMMON)/make/target_library_CLI.mk clean
	@printf "target_kmod.mk ----------------------------------------\n"
	@$(MAKE) -f $(RT_INCOMMON)/make/target_kmod.mk clean


