# Tests needing a ROOT instance
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} /usr/share/root/cmake)
find_package(ROOT)

if (ROOT_FOUND)
  include_directories(${ROOT_INCLUDE_DIR})
  link_directories(${ROOT_LIBRARY_DIR})
  
  if(${ROOT_VERSION} LESS 6.0)
    set(GCC_COMPILE_FLAGS "-Wno-shadow -fPIC")
  else()
    set(GCC_COMPILE_FLAGS "-Wno-shadow -fPIC -std=c++11")
  endif()
  add_definitions(${GCC_COMPILE_FLAGS})
endif()

function(ADD_TEST exec)
  add_executable(${exec} ${PROJECT_SOURCE_DIR}/test/${exec}.cpp $<TARGET_OBJECTS:reader_lib>)
  target_link_libraries(${exec} ${ROOT_LIBRARIES} ${ROOT_COMPONENT_LIBRARIES})
  set_property(TARGET ${exec} PROPERTY EXCLUDE_FROM_ALL true)
endfunction()

if (ROOT_FOUND)
  add_test(reader)
  add_test(reader_channelid)
  add_test(reader_oneedge)
  add_test(errors)
  #add_test(reader_2boards)
  add_test(write_tree_sorted)
endif()

