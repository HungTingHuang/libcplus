# Comment/uncomment the following line to enable/disable debugging
DEBUG 				= n
UNITTEST			= n
DEVELOP				= n
#
include cplusdef.mk

BUILD_DIR			?= $(shell pwd)
include libc.mk

ifeq (${DEVELOP}, y)
include unittest.mk
endif
