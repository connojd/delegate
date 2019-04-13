cmake_minimum_required(VERSION 3.13)
project(delegate)

add_executable(test-inher)

target_sources(test-inher PRIVATE ${PROJECT_SOURCE_DIR}/inheritance/test.cpp)
target_compile_features(test-inher PRIVATE cxx_std_17)
target_include_directories(test-inher PRIVATE ${PROJECT_SOURCE_DIR}/inheritance)
