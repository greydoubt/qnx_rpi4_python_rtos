ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

define PINFO
PINFO DESCRIPTION = event_loop
endef
INSTALLDIR=
NAME=event_loop
USEFILE=

EXTRA_INCVPATH=$(PROJECT_ROOT)/../include
#EXTRA_LIBVPATH=
#LIBS=
DEBUG=-g

include $(MKFILES_ROOT)/qtargets.mk
