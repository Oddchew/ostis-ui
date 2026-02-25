add_library(httplib INTERFACE) 
set_target_properties(httplib PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(httplib INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/cpp-httplib/)
