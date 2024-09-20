cmake_minimum_required(VERSION 3.8)

project(cppnat)

set(CMAKE_CXX_STANDARD 20)

if(WIN32)
    add_compile_definitions(_WIN32_WINNT=0x0601)
endif()

add_library(CPPNAT_LIBRARY STATIC 
    src/log.cpp
    src/message.cpp
    src/proxy_socket.cpp
    src/session.cpp
)

target_include_directories(CPPNAT_LIBRARY
    PUBLIC 
        ${PROJECT_SOURCE_DIR}/third_party/asio/asio/include
        ${PROJECT_SOURCE_DIR}/third_party/spdlog/include/
        ${PROJECT_SOURCE_DIR}/third_party/jsonserializer/include/
)

if (UNIX)
    target_link_libraries(CPPNAT_LIBRARY pthread)
endif()

if (MINGW)
    target_link_libraries(CPPNAT_LIBRARY ws2_32 mswsock)
endif()

add_executable(SERVER src/server.cpp src/server/main.cpp)

set_target_properties(SERVER PROPERTIES OUTPUT_NAME "server")
if(WIN32)
    set_target_properties(SERVER PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY $<TARGET_FILE_DIR:SERVER>)
else()
    set_target_properties(SERVER PROPERTIES RUNTIME_OUTPUT_DIRECTORY  ${CMAKE_SOURCE_DIR}/bin/)
endif()
add_custom_command(TARGET SERVER POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/bin/server.json $<TARGET_FILE_DIR:SERVER>)

target_link_libraries(SERVER
    PRIVATE 
    CPPNAT_LIBRARY
)

add_executable(CLIENT src/client.cpp src/client/main.cpp)

target_link_libraries(CLIENT
    PRIVATE 
    CPPNAT_LIBRARY
)

set_target_properties(CLIENT PROPERTIES OUTPUT_NAME "client")
if(WIN32)
    set_target_properties(CLIENT PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY $<TARGET_FILE_DIR:CLIENT>)
else()
    set_target_properties(CLIENT PROPERTIES RUNTIME_OUTPUT_DIRECTORY  ${CMAKE_SOURCE_DIR}/bin/)
endif()
add_custom_command(TARGET CLIENT POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/bin/client.json $<TARGET_FILE_DIR:CLIENT>)