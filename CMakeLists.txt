cmake_minimum_required(VERSION 3.10)
project(cplus VERSION 2.0.0) # CMAKE_PROJECT_VERSION

# execute_process (COMMAND bash -c "pwd" OUTPUT_VARIABLE CURRENT_PATH)
set(ROOT_PATH ${CMAKE_CURRENT_SOURCE_DIR})
string(CONCAT CPLUS_HEADER ${CMAKE_CURRENT_SOURCE_DIR} "/include")

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	include(GNUInstallDirs)
endif()

# fix "make[2]: warning:  Clock skew detected.  Your build may be incomplete."
# string()
# execute_process(COMMAND bash -c "find ./c_src -type f | xargs touch")

add_subdirectory(c_src)
add_subdirectory(test)
