cmake_minimum_required(VERSION 3.31.4)

project(
  sample
  DESCRIPTION "SAMPLE DESCRIPTION"
  LANGUAGES C)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(lib)

add_executable(${PROJECT_NAME} bin/main.c)

target_link_libraries(${PROJECT_NAME} PRIVATE samplelib)

target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/lib) 
