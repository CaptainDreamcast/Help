include Makefile.common
include $(KOS_BASE)/addons/prism/Makefile.commondc

all: complete

actions_user:
	cp -r assets/* filesystem

clean_user:
