cmake_minimum_required(VERSION 4.0)
project(web_server)

set(CMAKE_CXX_STANDARD 20)

add_executable(web_server src/main.cpp
        src/Connection.h
        src/HttpRequest.h
        src/Server.h
        src/HttpResponse.h
        src/ResponseFile.h)
