cmake_minimum_required(VERSION 3.27)

project(
  webserver
  VERSION 1.0.0
  LANGUAGES C)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/install" CACHE PATH "Installation prefix" FORCE)
endif()

find_package(HttpParser REQUIRED)

add_subdirectory(lib)

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    add_executable(${PROJECT_NAME} bin/main.c)
    file(COPY ${CMAKE_SOURCE_DIR}/assets/ DESTINATION ${CMAKE_BINARY_DIR}/)
    target_link_libraries(${PROJECT_NAME} PRIVATE HttpServer)
endif()
