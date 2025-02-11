ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

define PINFO
PINFO DESCRIPTION = mmap_demo
endef
INSTALLDIR=
NAME=mmap_demo
USEFILE=

#EXTRA_INCVPATH=
#EXTRA_LIBVPATH=
#LIBS=
DEBUG=-g

include $(MKFILES_ROOT)/qtargets.mk
