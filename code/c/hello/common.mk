ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

NAME=hello
USEFILE=

include $(MKFILES_ROOT)/qtargets.mk
