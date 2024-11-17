LIBRARY_NAME 			= libcplus

CROSS_COMPILER			= /usr/bin/
CC 						= $(CROSS_COMPILER)gcc
# CC 						= $(CROSS_COMPILER)g++
AR						= $(CROSS_COMPILER)ar
LD						= $(CROSS_COMPILER)ld

CFLAGS 					+= -I$(HDR_DIR)
# CFLAGS					+= -std=gnu99 # C99
# CFLAGS					+= -std=gnu17 # C17
CFLAGS					+= -std=gnu2x # C23
# CFLAGS					+= -std=gnu++17 # C++17
# CFLAGS					+= -std=gnu++2b # C++23
ifeq ($(DEBUG), y)
CFLAGS					+= -g
CFLAGS					+= -D__CPLUS_MEM_MANAGER__
endif
CFLAGS					+= -Wall

LIBS 					+= -L.
LIBS 					+= -L/usr/local/lib
LIBS					+= -Wl,-rpath=/usr/lib/arm-linux-gnueabihf
LIBS					+= -Wl,-rpath=/usr/local/lib
LIBS					+= -Wl,-rpath=/usr/lib
LIBS					+= -Wl,-rpath=/lib
LIBS 					+= -lpthread
LIBS 					+= -lrt
LIBS 					+= -latomic

.PHONY: dylib install_dynamic_library uninstall_dynamic_library clean
all:
dylib: build_dynamic_library clean
install: install_dynamic_library
uninstall: uninstall_dynamic_library

$(LIB_OBJS): $(SRC_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) ${CFLAGS} -fPIC -c $< -o $@ ${LIBS}

build_dynamic_library: $(LIB_OBJS)
	$(CC) ${CFLAGS} -shared -Wl,-soname,$(LIBRARY_NAME).so -o $(LIBRARY_NAME).so.$(LIB_VER) $(LIB_OBJS) ${LIBS}

install_dynamic_library:
	@sudo install -m 777 -d $(INSTALL_HDR_DIR)/cplus
	@sudo install -m 777 $(HDR_DIR)/* $(INSTALL_HDR_DIR)/cplus
	@sudo install -m 777 $(LIBRARY_NAME).so.$(LIB_VER) $(INSTALL_LIB_DIR)
	@sudo ln -sf $(LIBRARY_NAME).so.$(LIB_VER) $(INSTALL_LIB_DIR)/$(LIBRARY_NAME).so.$(MAJOR).$(MINOR)
	@sudo ln -sf $(LIBRARY_NAME).so.$(LIB_VER) $(INSTALL_LIB_DIR)/$(LIBRARY_NAME).so.$(MAJOR)
	@sudo ln -sf $(LIBRARY_NAME).so.$(LIB_VER) $(INSTALL_LIB_DIR)/$(LIBRARY_NAME).so
	@sudo ldconfig

uninstall_dynamic_library:
	@sudo rm -rf $(INSTALL_HDR_DIR)/cplus
	@sudo rm -f $(INSTALL_LIB_DIR)/$(LIBRARY_NAME).so.$(LIB_VER)
	@sudo rm -f $(INSTALL_LIB_DIR)/$(LIBRARY_NAME).so.$(MAJOR).$(MINOR)
	@sudo rm -f $(INSTALL_LIB_DIR)/$(LIBRARY_NAME).so.$(MAJOR)
	@sudo rm -f $(INSTALL_LIB_DIR)/$(LIBRARY_NAME).so

clean:
	@rm $(SRC_DIR)/*.o