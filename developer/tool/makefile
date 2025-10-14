
RT-INCOMMON:=$(REPO_HOME)/tool_shared/third_party/RT-project-share/release

include $(RT-INCOMMON)/make/environment_RT_0

# To compile the example directory uncomment the following assignments:
SRCDIR_List=cc
CFLAGS+=-Icc

CFLAGS+= -include "$(RT-INCOMMON)/make/RT_0.h"
LINKFLAGS+= -l$(PROJECT)
LIBFILE=$(LIBDIR)/lib$(PROJECT).a

include $(RT-INCOMMON)/make/targets_developer
-include $(DEPFILE)

