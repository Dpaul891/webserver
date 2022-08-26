cmake_minimum_required(VERSION 3.16)
project(webserver)

# C++17
set(CMAKE_CXX_STANDARD 17)

# 添加文件
include_directories(include)

#添加源文件
aux_source_directory(./src DIR_SRCS)

# 所有需要编译的可执行文件
add_executable(client src/client.cpp src/warp.cpp)
add_executable(server src/server.cpp src/warp.cpp)