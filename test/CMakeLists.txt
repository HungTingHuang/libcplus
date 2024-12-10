##
set(TARGET unittest_cplus_run)
string(CONCAT GOOGLETEST ${CMAKE_CURRENT_SOURCE_DIR} "/googletest")
string(CONCAT CPLUS ${CMAKE_CURRENT_SOURCE_DIR} "build/c_src")

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CPLUS_HEADER} ${GOOGLETEST})
link_directories(${GOOGLETEST} ${CPLUS})

file(GLOB SRCS CONFIGURE_DEPENDS "*.cc")
add_executable(${TARGET} ${SRCS})

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	list(APPEND CFLAGS -std=gnu++2b)
endif()

list(APPEND STATIC_LINK_FILES libgmock_main.a)
list(APPEND STATIC_LINK_FILES libgmock.a)
list(APPEND STATIC_LINK_FILES libgtest_main.a)
list(APPEND STATIC_LINK_FILES libgtest.a)
target_link_libraries(${TARGET} ${STATIC_LINK_FILES})

target_compile_options(${TARGET} PRIVATE ${CFLAGS})
target_compile_options(${TARGET} PRIVATE ${LDFLAGS})

list(APPEND DYNMAIC_LINK_FILES cplus)
target_link_libraries(${TARGET} ${DYNMAIC_LINK_FILES})


install(TARGETS ${TARGET} DESTINATION exe)