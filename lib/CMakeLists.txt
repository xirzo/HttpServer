add_library(HttpServer STATIC ./src/server.c ./src/routes.c)

target_link_libraries(HttpServer PRIVATE HttpParser::http_parser)

target_include_directories(HttpServer PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

set_target_properties(HttpServer PROPERTIES
    VERSION 1.0.0
    SOVERSION 1
    PUBLIC_HEADER "include/server.h;include/routes.h"
)

install(TARGETS HttpServer
    EXPORT HttpServerTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include
)

install(DIRECTORY include/ DESTINATION include
    FILES_MATCHING PATTERN "*.h"
)

install(EXPORT HttpServerTargets
    FILE HttpServerTargets.cmake
    NAMESPACE HttpServer::
    DESTINATION lib/cmake/HttpServer
)

include(CMakePackageConfigHelpers)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/HttpServerConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/HttpServerConfig.cmake"
    INSTALL_DESTINATION lib/cmake/HttpServer
)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/HttpServerConfigVersion.cmake"
    VERSION 1.0.0
    COMPATIBILITY AnyNewerVersion
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/HttpServerConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/HttpServerConfigVersion.cmake"
    DESTINATION lib/cmake/HttpServer
)
