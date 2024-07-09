# Comment/uncomment the following line to enable/disable debugging
DEBUG 				= y
UNITTEST			= n
DEVELOP				= y
#
LIBRARY_NAME 			= libcplus
# Change according to your files
include cplusdef.mk
BUILD_DIR			?= .
SRC 				:= $(addprefix $(SRC_DIR)/,$(SOURCES:=.c))

LIBS   				+= -lrt -pthread -latomic
LIBS				+= -Wl,-rpath=/usr/lib/arm-linux-gnueabihf

# include ${TOOLDIR}/rules/Rules.make.libs

ifeq (${DEVELOP}, y)
include unittest.mk
endif
