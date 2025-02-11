ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

define PINFO
PINFO DESCRIPTION = led_client
endef
INSTALLDIR=
NAME=led_client
USEFILE=

EXTRA_INCVPATH=$(PROJECT_ROOT)/../include
#EXTRA_LIBVPATH=
#LIBS=
DEBUG=-g

include $(MKFILES_ROOT)/qtargets.mk
