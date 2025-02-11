ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

define PINFO
PINFO DESCRIPTION = gpio_interrupt
endef
INSTALLDIR=
NAME=gpio_interrupt
USEFILE=

EXTRA_INCVPATH=$(PROJECT_ROOT)/../include
#EXTRA_LIBVPATH=
#LIBS=
DEBUG=-g

include $(MKFILES_ROOT)/qtargets.mk
