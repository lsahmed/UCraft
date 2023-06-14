#build-essential has to be installed
cmake_minimum_required(VERSION 3.16)

# set the project name
project(UCraft)

include(${CMAKE_SOURCE_DIR}/3rdparty/cmake/mbedtls/mbedtls.cmake)
include(${CMAKE_SOURCE_DIR}/3rdparty/cmake/lwjson/lwjson.cmake)


set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS True)

add_subdirectory(src)