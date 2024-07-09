UNITTEST_NAME 			= unittest_run
UNITTEST_SRC  			= unittest
INSTALL_LIB_DIR			= /usr/local/lib
INSTALL_HDR_DIR			= /usr/local/include

# CROSS_COMPILER			= /usr/bin/
CROSS_COMPILER			= /opt/build_env/bullseye/crosscc/bin/arm-hard-linux-gnueabihf-
CC 						= $(CROSS_COMPILER)gcc
AR						= $(CROSS_COMPILER)ar
LD						= $(CROSS_COMPILER)ld

include cplusdef.mk
LIB_OBJS				:= $(addprefix $(SRC_DIR)/,$(SOURCES:=.o))
CFLAGS					+= -g
CFLAGS					+= -Wall
ifeq ($(UNITTEST), y)
CFLAGS					+= -D__CPLUS_UNITTEST__
endif
ifeq ($(DEBUG), y)
CFLAGS					+= -D__CPLUS_MEM_MANAGER__
endif

LIBS 					+= -L.
LIBS 					+= -L/usr/local/lib
LIBS					+= -Wl,-rpath=/usr/local/lib
LIBS					+= -Wl,-rpath=/usr/lib
LIBS					+= -Wl,-rpath=/lib

LIBS 					+= -pthread
LIBS 					+= -lrt

.PHONY: lib test build_library install_library build_unittest clean

all:

lib: build_library install_library clean

test: build_library install_library build_unittest clean

build_library: $(LIB_OBJS)
	$(CC) ${CFLAGS} -shared -Wl,-soname,$(LIBRARY_NAME).so -o $(LIBRARY_NAME).so.$(LIB_VER) $(LIB_OBJS) ${LIBS}
$(LIB_OBJS): $(SRC_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) ${CFLAGS} -fPIC -c $< -o $@ ${LIBS}

build_unittest: $(UNITTEST_SRC).o
	$(CC) ${CFLAGS} -o $(UNITTEST_NAME) $(UNITTEST_SRC).o ${LIBS} -lcplus
$(UNITTEST_SRC).o:
	$(CC) ${CFLAGS} -c $(SRC_DIR)/$(UNITTEST_SRC).c

install_library:
	@sudo install -m 777 -d $(INSTALL_HDR_DIR)/cplus
	@sudo install -m 777 $(HDR_DIR)/* $(INSTALL_HDR_DIR)/cplus
	@sudo cp $(LIBRARY_NAME).so.$(LIB_VER) $(INSTALL_LIB_DIR)
	@sudo ln -sf $(LIBRARY_NAME).so.$(LIB_VER) $(INSTALL_LIB_DIR)/$(LIBRARY_NAME).so.$(MAJOR).$(MINOR)
	@sudo ln -sf $(LIBRARY_NAME).so.$(LIB_VER) $(INSTALL_LIB_DIR)/$(LIBRARY_NAME).so.$(MAJOR)
	@sudo ln -sf $(LIBRARY_NAME).so.$(LIB_VER) $(INSTALL_LIB_DIR)/$(LIBRARY_NAME).so
	@sudo ldconfig

clean:
	@rm $(SRC_DIR)/*.o
	@[ ! -f ./unittest.o ] || rm ./unittest.o
