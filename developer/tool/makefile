# developer/tool/makefile — Orchestrator (Hybrid)

ifndef REPO_HOME
  $(error REPO_HOME is not set; source your project env (tool_shared/bespoke/env) first)
endif

RT_INCOMMON := $(REPO_HOME)/tool_shared/third_party/RT-project-share/release
include $(RT_INCOMMON)/make/environment_RT_1.mk

.PHONY: all usage lib_cli kmod clean check-pwd
all: lib_cli kmod

usage:
	@printf "Usage: make [usage|lib_cli|kmod|clean|check-pwd]\n"; exit 2

# Build user-space static lib + CLI (cli target builds lib first)
lib_cli:
	@$(MAKE) -f $(RT_INCOMMON)/make/target_lib_cli.mk cli

# Build all kernel modules
kmod:
	@$(MAKE) -f $(RT_INCOMMON)/make/target_kmod.mk kmod

clean:
	@$(MAKE) -f $(RT_INCOMMON)/make/target_lib_cli.mk clean
	@$(MAKE) -f $(RT_INCOMMON)/make/target_kmod.mk clean

check-pwd:
	@if [ "$(CURDIR)" != "$(REPO_HOME)/developer" ]; then \
	  printf "warn: CURDIR=%s (expected %s)\n" "$(CURDIR)" "$(REPO_HOME)/developer"; \
	fi
