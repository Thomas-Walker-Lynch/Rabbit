
RT-INCOMMON:=$(REPO_HOME)/release

include $(RT-INCOMMON)/make/environment_RT_0

CFLAGS+=-Werror -include "$(RT-INCOMMON)/make/RT_0.h"
LINKFLAGS+= -l$(PROJECT)
LIBFILE=$(LIBDIR)/lib$(PROJECT).a

include $(RT-INCOMMON)/make/targets_developer
-include $(DEPFILE)
