# WebSocket definition
file(GLOB ws_sources WebSocket/*.cpp WebSocket/*/*.cpp)
include_directories("WebSocket")

add_library(ws_lib OBJECT ${ws_sources})
