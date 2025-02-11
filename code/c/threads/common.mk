ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

define PINFO
PINFO DESCRIPTION = threads
endef
INSTALLDIR=
NAME=threads
USEFILE=

#EXTRA_INCVPATH=
#EXTRA_LIBVPATH=
#LIBS=
DEBUG=-g

include $(MKFILES_ROOT)/qtargets.mk
