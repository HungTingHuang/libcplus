MAJOR				= 1
MINOR				= 0
PATCH				= 0
LIB_VER				:= $(MAJOR).$(MINOR).$(PATCH)

INSTALL_LIB_DIR		= /usr/local/lib
INSTALL_HDR_DIR		= /usr/local/include

SOURCES 			= helper
SOURCES     		+= atomic
SOURCES				+= memmgr
SOURCES 			+= sys
SOURCES     		+= systime
SOURCES 			+= sharedmem
SOURCES 			+= semaphore
SOURCES				+= mutex
SOURCES 			+= rwlock
SOURCES 			+= pevent
SOURCES				+= mempool
SOURCES 			+= llist
SOURCES 			+= task
SOURCES 			+= taskpool
SOURCES 			+= syslog
SOURCES 			+= data
SOURCES 			+= socket
SOURCES 			+= ipc_server
SOURCES 			+= file
SOURCES 			+= event_server
SOURCES 			+= cplus

SRC_DIR 			?= $(BUILD_DIR)/c_src
HDR_DIR 			?= $(BUILD_DIR)/include
SRC 				:= $(addprefix $(SRC_DIR)/,$(SOURCES:=.c))


