ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

define PINFO
PINFO DESCRIPTION = gpio_map
endef
INSTALLDIR=
NAME=gpio_map
USEFILE=

EXTRA_INCVPATH=$(PROJECT_ROOT)/../include
#EXTRA_LIBVPATH=
#LIBS=
DEBUG=-g

include $(MKFILES_ROOT)/qtargets.mk
