cmake_minimum_required(VERSION 3.12)
project(CampusConnectServer CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(backend)

set(SOURCES
    backend/main.cpp
    backend/database.cpp
    backend/server.cpp
    backend/cli.cpp
)

add_executable(server ${SOURCES})

if (WIN32)
    target_link_libraries(server ws2_32)
endif()
