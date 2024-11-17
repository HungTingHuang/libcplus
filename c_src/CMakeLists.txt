##
set(TARGET libcplus)
include_directories(../include)

file(GLOB SRCS CONFIGURE_DEPENDS "*.h" "*.c")
add_library(${TARGET} SHARED ${SRCS})

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")

    list(APPEND CFLAGS -std=gnu2x) # C23
    list(APPEND CFLAGS -Wall)
    list(APPEND CFLAGS -Wextra)
    # list(APPEND CFLAGS -Wpedantic)
    list(APPEND CFLAGS -Werror)

    ## Set LDFLAGS
    list(APPEND LDFLAGS -lpthread)
    list(APPEND LDFLAGS -latomic)
    list(APPEND LDFLAGS -lrt)
endif()

target_compile_options(${TARGET} PRIVATE ${CFLAGS})
target_compile_options(${TARGET} PRIVATE ${LDFLAGS})

