cmake_minimum_required(VERSION 4.0)
project(web_server)

set(CMAKE_CXX_STANDARD 20)

set(HEADERS
        src/http/HttpRequest.hpp
        src/http/HttpResponse.hpp
        src/net/Server.hpp
        src/net/Connection.hpp
        src/net/ResponseFile.hpp
)

set(SOURCES
        src/http/HttpRequest.cpp
        src/http/HttpResponse.cpp
        src/net/Server.cpp
        src/net/Connection.cpp
        src/net/ResponseFile.cpp
)

add_executable(${PROJECT_NAME} src/main.cpp ${SOURCES} ${HEADERS})

target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra)