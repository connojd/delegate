cmake_minimum_required(VERSION 3.13)
project(delegate)

set(CMAKE_BUILD_TYPE Release)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(test)

target_sources(test PRIVATE ${PROJECT_SOURCE_DIR}/placement/test.cpp)
target_compile_features(test PRIVATE cxx_std_17)
target_compile_options(test PRIVATE -msse -msse2 -msse3 -msse4)
target_include_directories(test PRIVATE ${PROJECT_SOURCE_DIR}/placement)
