#find lwjson in the system
find_package(lwjson QUIET)
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()
if(lwjson_FOUND)
    message(STATUS "Found lwjson: ${lwjson_INCLUDE_DIRS} ${lwjson_LIBRARIES}")
else()
    message(STATUS "lwjson not found, cloning and will build from source")
    find_package(Git REQUIRED)
    execute_process(
        COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive 3rdparty/lwjson
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )
    add_library(lwjson STATIC 3rdparty/lwjson/lwjson/src/lwjson/lwjson.c)
    target_include_directories(lwjson PUBLIC 3rdparty/lwjson/lwjson/src/include/)
    target_compile_definitions(lwjson PUBLIC LWJSON_IGNORE_USER_OPTS)
endif()
