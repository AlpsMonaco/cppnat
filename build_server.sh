#!/bin/bash
time=$(date "+%Y-%m-%d %H:%M:%S")
echo "build server start at ${time}"

g++ \
-std=c++2a \
-obin/server \
-O2 \
src/message.cpp \
src/proxy_socket.cpp \
src/server.cpp \
src/session.cpp \
src/log.cpp \
src/server/main.cpp \
-Ithird_party/asio/asio/include \
-Ithird_party/spdlog/include \
-Ithird_party/jsonserializer/include \
-lpthread

time=$(date "+%Y-%m-%d %H:%M:%S")
echo "build server end at ${time}"
