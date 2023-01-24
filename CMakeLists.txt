cmake_minimum_required(VERSION 3.23)
project(BlogWebServer)

set(CMAKE_CXX_STANDARD 14)

include_directories(MainServer)
include_directories(Lib/sqlite)

message("link to sqlite3")
add_library(sqlite3 Lib/sqlite/sqlite3.dll)
set_target_properties(sqlite3 PROPERTIES LINKER_LANGUAGE C)

file(GLOB HEADERS "MainServer/*.h")
file(GLOB SOURCES "MainServer/*.cpp")

add_executable(BlogWebServer
        ${SOURCES}
        ${HEADERS})
