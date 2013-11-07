cmake_minimum_required (VERSION 2.8)
project (OS-Camtroller)

find_package( OpenCV REQUIRED )

set(warnings "-Wall -Wextra -Werror")
add_definitions(-std=c++0x)
set(INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include)
set(SRC_DIR ${PROJECT_SOURCE_DIR}/src)
set(BIN_DIR ${PROJECT_SOURCE_DIR}/bin)
set(SRC_FILES ${SRC_DIR}/main.cpp)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BIN_DIR})
include_directories(${INCLUDE_DIR})
add_executable(OS-Camtroller ${SRC_FILES})
target_link_libraries(OS-Camtroller ${OpenCV_LIBS})
