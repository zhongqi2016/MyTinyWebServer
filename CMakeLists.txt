cmake_minimum_required(VERSION 3.20)
project(WebServer)

set(CMAKE_CXX_STANDARD 20)

add_executable(WebServer main.cpp WebServer.cpp WebServer.h http.cpp http.h ThreadPool.cpp ThreadPool.h EpollControl.cpp EpollControl.h)
