##
set(TARGET cplus)

include_directories(${CPLUS_HEADER} ${CMAKE_CURRENT_SOURCE_DIR})

file(GLOB SRCS CONFIGURE_DEPENDS "*.c")
add_library(${TARGET} SHARED ${SRCS})
# set_target_properties(${TARGET} PROPERTIES
# 	SOVERSION ${CMAKE_PROJECT_VERSION}
# )
set_source_files_properties(${SRCS} PROPERTIES LANGUAGE CXX)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	# list(APPEND CFLAGS -std=gnu2x)
	list(APPEND CFLAGS -std=gnu++2b)
	list(APPEND CFLAGS -Wall)
	list(APPEND CFLAGS -Wextra)
	# list(APPEND CFLAGS -Wpedantic)
	list(APPEND CFLAGS -Werror)
	list(APPEND LDFLAGS -lpthread)
	list(APPEND LDFLAGS -latomic)
	list(APPEND LDFLAGS -lrt)
endif()

target_compile_options(${TARGET} PRIVATE ${CFLAGS})
target_compile_options(${TARGET} PRIVATE ${LDFLAGS})


# set(CMAKE_INSTALL_MODE REL_SYMLINK)
install(TARGETS ${TARGET} DESTINATION lib)
# set_target_properties(${TARGET} PROPERTIES VERSION ${CMAKE_PROJECT_VERSION} SOVERSION 0)
