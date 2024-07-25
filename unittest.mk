UNITTEST_NAME 			= unittest_run
UNITTEST_SRC  			= unittest
INSTALL_LIB_DIR			= /usr/local/lib
INSTALL_HDR_DIR			= /usr/local/include

CROSS_COMPILER			= /usr/bin/
# CROSS_COMPILER			= /opt/build_env/bullseye/crosscc/bin/arm-hard-linux-gnueabihf-
CC 						= $(CROSS_COMPILER)gcc
# CC 						= $(CROSS_COMPILER)g++
AR						= $(CROSS_COMPILER)ar
LD						= $(CROSS_COMPILER)ld

include cplusdef.mk
LIB_OBJS				:= $(addprefix $(SRC_DIR)/,$(SOURCES:=.o))

CFLAGS					+= -g
CFLAGS					+= -Wall
# CFLAGS					+= -std=gnu99 # C99
# CFLAGS					+= -std=gnu17 # C17
CFLAGS					+= -std=gnu2x # C23
# CFLAGS					+= -std=gnu++17 # C++23
# CFLAGS					+= -std=gnu++2b # C++23

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

.PHONY: lib test build_library install_library build_unittest build_unittest_one clean

all:
lib: build_library install_library clean
ifeq ($(UNITTEST), y)
test: build_library install_library build_unittest clean
test_pack: build_unittest_pack clean
endif
install: install_library
uninstall: uninstall_library

$(LIB_OBJS): $(SRC_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) ${CFLAGS} -fPIC -c $< -o $@ ${LIBS}

$(UNITTEST_SRC).o:
	$(CC) ${CFLAGS} -c $(SRC_DIR)/$(UNITTEST_SRC).c

build_library: $(LIB_OBJS)
	$(CC) ${CFLAGS} -shared -Wl,-soname,$(LIBRARY_NAME).so -o $(LIBRARY_NAME).so.$(LIB_VER) $(LIB_OBJS) ${LIBS}

build_unittest: $(UNITTEST_SRC).o
	$(CC) ${CFLAGS} -o $(UNITTEST_NAME) $(UNITTEST_SRC).o ${LIBS} -lcplus

build_unittest_pack: $(LIB_OBJS) $(UNITTEST_SRC).o
	$(CC) ${CFLAGS} -o $(UNITTEST_NAME)_pack $(LIB_OBJS) $(UNITTEST_SRC).o ${LIBS}

install_library:
	@sudo install -m 777 -d $(INSTALL_HDR_DIR)/cplus
	@sudo install -m 777 $(HDR_DIR)/* $(INSTALL_HDR_DIR)/cplus
	@sudo install -m 777 $(LIBRARY_NAME).so.$(LIB_VER) $(INSTALL_LIB_DIR)
	@sudo ln -sf $(LIBRARY_NAME).so.$(LIB_VER) $(INSTALL_LIB_DIR)/$(LIBRARY_NAME).so.$(MAJOR).$(MINOR)
	@sudo ln -sf $(LIBRARY_NAME).so.$(LIB_VER) $(INSTALL_LIB_DIR)/$(LIBRARY_NAME).so.$(MAJOR)
	@sudo ln -sf $(LIBRARY_NAME).so.$(LIB_VER) $(INSTALL_LIB_DIR)/$(LIBRARY_NAME).so
	@sudo ldconfig

uninstall_library:
	@sudo rm -rf $(INSTALL_HDR_DIR)/cplus
	@sudo rm -f $(INSTALL_LIB_DIR)/$(LIBRARY_NAME).so.$(LIB_VER)
	@sudo rm -f $(INSTALL_LIB_DIR)/$(LIBRARY_NAME).so.$(MAJOR).$(MINOR)
	@sudo rm -f $(INSTALL_LIB_DIR)/$(LIBRARY_NAME).so.$(MAJOR)
	@sudo rm -f $(INSTALL_LIB_DIR)/$(LIBRARY_NAME).so

clean:
	@rm $(SRC_DIR)/*.o
	@[ ! -f ./unittest.o ] || rm ./unittest.o