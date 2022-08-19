#!/bin/bash
time=$(date "+%Y-%m-%d %H:%M:%S")
echo "build client start at ${time}"

g++ \
-std=c++2a \
-obin/client \
-O2 \
src/message.cpp \
src/proxy_socket.cpp \
src/client.cpp \
src/session.cpp \
src/log.cpp \
src/client/main.cpp \
-Ithird_party/asio/asio/include \
-Ithird_party/spdlog/include \
-Ithird_party/jsonserializer/include \
-lpthread

time=$(date "+%Y-%m-%d %H:%M:%S")
echo "build client end at ${time}"