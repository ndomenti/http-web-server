cmake_minimum_required(VERSION 4.0)
project(web_server)

set(CMAKE_CXX_STANDARD 20)

add_executable(web_server src/main.cpp

        src/http/HttpRequest.hpp
        src/http/HttpRequest.cpp

        src/http/HttpResponse.hpp
        src/http/HttpResponse.cpp

        src/net/Server.hpp
        src/net/Server.cpp

        src/net/Connection.hpp
        src/net/Connection.cpp

        src/net/ResponseFile.hpp
        src/net/ResponseFile.cpp)
