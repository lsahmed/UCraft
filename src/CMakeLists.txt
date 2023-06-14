add_executable(${PROJECT_NAME} main.c UCraft.c socketio.c player.c c2s.c s2c.c util.c log.c blobs/codec.c encryption.c https.c game.c)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/include/UCraft)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR})

set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "-Werror -Wall -Wextra -Wno-unused-parameter")
#set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "")
target_link_libraries(${PROJECT_NAME} PRIVATE m) #math library
target_link_libraries(${PROJECT_NAME} PRIVATE mbedtls)
target_link_libraries(${PROJECT_NAME} PRIVATE lwjson)

#run the size command afterwards
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND size --format=berkeley ${PROJECT_NAME})
