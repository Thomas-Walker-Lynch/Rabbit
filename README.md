# Harmony — RT project skeleton

Tiny, opinionated starter project skeleton that we use across RT projects independent of language being coded.

Pick a role, source the env, build your thing, then release.

## Roles (source these, don’t execute)
- `env_developer`  — dev workflow
- `env_tester`     — test + repro
- `env_toolsmith`  — shared tools + env wiring

Developers work under `developer/`, testers under `tester/`, toolsmiths wire `tool_shared/` and env scripts.

## Layout (why it exists)
- `document/`                        — project docs (+ RT conventions in org)
- `developer/`                       — dev code, experiments, dev-specific docs/tools
- `tester/`                          — tests, fixtures, repro steps
- `tool_shared/`                     — shared tools/env for all roles
- `tool_shared/third_party/`         — third-party tools
- `tool_shared/third_party/python/`  — your venv lives here (not committed)
- `release/`                         — publishable artifacts
- `tmp/`                             — scratch (gitignored)

Empty directories are tracked with `.githolder` (kept out of release archives).

## Quick start
```bash
# choose a role (must be sourced)
source ./env_developer   # or env_tester / env_toolsmith

# create the Python venv under tool_shared/third_party/python/ (literally 'python' instead of 'venv'
./scripts/python_venv_bootstrap.sh

# re-enter later
source ./env_developer

# where used

In public projects, this structure has been used with Python, Java, C, C++, and Lisp projects.

Note the related https://github.com/Thomas-Walker-Lynch/RT-project-share project. It has the generic makefile used on C/C++ projects and other shared tools.  Note the project https://github.com/Thomas-Walker-Lynch/RT_gcc for a more fully featured cpp.  Note the projects https://github.com/Thomas-Walker-Lynch/Mosaic, and https://github.com/Thomas-Walker-Lynch/Mosaic for Java examples of this project skeleton being used for a Java testing and dependency grapph build tool, respectively.

