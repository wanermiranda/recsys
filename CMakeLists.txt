cmake_minimum_required(VERSION 3.3)
project(TP1_Recsys)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
FIND_PACKAGE ( Threads REQUIRED )

set(SOURCE_FILES main.cpp CSVReader.cpp CSVReader.h StringUtils.cpp StringUtils.h Predictor.cpp Predictor.h
        ArrayUtils.h distances.h distances.cpp )
add_executable(TP1_Recsys ${SOURCE_FILES})
TARGET_LINK_LIBRARIES ( TP1_Recsys ${CMAKE_THREAD_LIBS_INIT} )
