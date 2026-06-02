cmake_minimum_required(VERSION 3.16)
project(EftpClient VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
if(MSVC)
    add_compile_options(/utf-8 /MP)
endif()

add_definitions(-D_HAS_STD_BYTE=0 -D_UNICODE -D_WINDOWS -DQT_DEPRECATED_WARNINGS)

set(OUTPUT_BIN_DIR "${CMAKE_SOURCE_DIR}/Output/Win32/Bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_BIN_DIR})

find_package(Qt5 REQUIRED COMPONENTS Core Network Gui Widgets Concurrent Sql Xml)
set(QT_VERSION_MAJOR 5)

add_subdirectory(QtLogger)
add_subdirectory(HttpCilent)
add_subdirectory(EftpClient)
