cmake_minimum_required(VERSION 3.17)
project(DA_Conference_Tool)

set(CMAKE_CXX_STANDARD 17)

include_directories(include)

add_executable(da_tool
    src/main.cpp
    src/Menu.cpp
    src/CSVParser.cpp
    src/FlowNetwork.cpp
    src/OutputWriter.cpp
    src/DisplayFormatter.cpp
)
