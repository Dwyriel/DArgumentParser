cmake_minimum_required(VERSION 3.25)

project(DArgumentParser VERSION 0.1 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(PROJECT_SOURCES
        DArgumentParser.cpp
        )

add_library(DArgumentParser STATIC ${PROJECT_SOURCES})