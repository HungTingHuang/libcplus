# Comment/uncomment the following line to enable/disable debugging
DEBUG 				= n
UNITTEST			= n
DEVELOP				= n
#
LIBRARY_NAME 			= libcplus
# Change according to your files
include cplusdef.mk
BUILD_DIR			?= .
SRC 				:= $(addprefix $(SRC_DIR)/,$(SOURCES:=.c))

LIBS   				+= -lrt -pthread -latomic
LIBS				+= -Wl,-rpath=/usr/lib/arm-linux-gnueabihf

ifneq (${DEVELOP}, y)
include ${TOOLDIR}/rules/Rules.make.libs
else
include unittest.mk
endif
