MAJOR			= 13
MINOR			= 0
PATCH			= 7
LIB_VER			:= $(MAJOR).$(MINOR).$(PATCH)

SOURCES 		= helper
SOURCES     	+= atomic
SOURCES			+= memmgr
SOURCES 		+= sys
SOURCES     	+= systime
SOURCES 		+= sharedmem
SOURCES 		+= semaphore
SOURCES			+= mutex
SOURCES 		+= rwlock
SOURCES 		+= pevent
SOURCES			+= mempool
SOURCES 		+= llist
SOURCES 		+= task
SOURCES 		+= taskpool
SOURCES 		+= syslog
SOURCES 		+= data
SOURCES 		+= socket
SOURCES 		+= ipc_server
SOURCES 		+= file
SOURCES 		+= cplus

SRC_DIR 		?= $(BUILD_DIR)/src
HDR_DIR 		?= $(BUILD_DIR)/cplus

CFLAGS 			+= -I$(HDR_DIR)
