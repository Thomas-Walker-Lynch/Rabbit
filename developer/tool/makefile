.SUFFIXES:
# developer/tool/makefile — Orchestrator (Hybrid)

RT_INCOMMON := $(REPO_HOME)/tool_shared/third_party/RT-project-share/release
include $(RT_INCOMMON)/make/environment_RT_1.mk

.PHONY: usage 
usage:
	@printf "Usage: make [usage|information|all|lib|cli|kmod|clean]\n"; exit 2

.PHONY: version 
version:
	@echo tool/makefile version 2.0
	@$(MAKE) -f $(RT_INCOMMON)/make/target_kmod.mk version
	@$(MAKE) -f $(RT_INCOMMON)/make/target_lib_cli.mk version

.PHONY: information
information:
	@printf "local ----------------------------------------\n"
	-@echo CURDIR='$(CURDIR)'
	@echo REPO_HOME="$(REPO_HOME)"
	@echo KMOD_BUILD_DIR="/lib/modules/$(shell uname -r)/build"
	@echo CURDIR="$(CURDIR)"
	@printf "target_lib_cli.mk ----------------------------------------\n"
	@$(MAKE) -f $(RT_INCOMMON)/make/target_lib_cli.mk information
	@printf "target_kmod.mk ----------------------------------------\n"
	@$(MAKE) -f $(RT_INCOMMON)/make/target_kmod.mk information

.PHONY: all
all: lib cli kmod

.PHONY: lib
lib:
	@$(MAKE) -f $(RT_INCOMMON)/make/target_lib_cli.mk lib

.PHONY: cli
cli:
	@$(MAKE) -f $(RT_INCOMMON)/make/target_lib_cli.mk cli

.PHONY: kmod
kmod:
	@$(MAKE) -f $(RT_INCOMMON)/make/target_kmod.mk kmod

.PHONY: clean
clean:
	@$(MAKE) -f $(RT_INCOMMON)/make/target_lib_cli.mk clean
	@$(MAKE) -f $(RT_INCOMMON)/make/target_kmod.mk clean


