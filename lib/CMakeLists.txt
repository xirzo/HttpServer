add_library(server STATIC ./src/server.c ./src/routes.c)

target_link_libraries(server PRIVATE HttpParser::http_parser)

target_include_directories(server PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
