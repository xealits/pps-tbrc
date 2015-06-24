# File reader
add_library(reader_lib OBJECT ${PROJECT_SOURCE_DIR}/src/FileReader.cpp)
add_executable(reader reader.cpp $<TARGET_OBJECTS:reader_lib>)
#set_property(TARGET reader PROPERTY EXCLUDE_FROM_ALL true)
