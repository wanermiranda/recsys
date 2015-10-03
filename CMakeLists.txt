cmake_minimum_required(VERSION 3.3)
project(TP1_Recsys)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES main.cpp CSVReader.cpp CSVReader.h StringUtils.cpp StringUtils.h)
add_executable(TP1_Recsys ${SOURCE_FILES})