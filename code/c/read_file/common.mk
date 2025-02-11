ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

define PINFO
PINFO DESCRIPTION = read_file
endef
INSTALLDIR=
NAME=read_file
USEFILE=

#EXTRA_INCVPATH=
#EXTRA_LIBVPATH=
#LIBS=
DEBUG=-g -O0

include $(MKFILES_ROOT)/qtargets.mk
