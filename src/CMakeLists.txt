file(GLOB_RECURSE CPP_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.cpp)
file(GLOB_RECURSE HEADER_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.h)
file(GLOB_RECURSE RESOURCE_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.json *.ini)

source_group("resource" FILES ${RESOURCE_FILES})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${CPP_FILES})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${HEADER_FILES})

add_executable(ctp-marketdata-collector ${CPP_FILES} ${HEADER_FILES})
target_include_directories(ctp-marketdata-collector PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(ctp-marketdata-collector
                      json-schema-validator
                      ${Boost_LIBRARIES}
                      ${MONGOC_STATIC_LIBRARIES}
                      libmongocxx
                      libbsoncxx
                      thostmduserapi)
target_compile_definitions(ctp-marketdata-collector
                           PRIVATE ${MONGOC_STATIC_DEFINITIONS}
                           PRIVATE MONGOCXX_STATIC
                           PRIVATE BSONCXX_STATIC)
# target_compile_options(ctp-marketdata-collector PRIVATE "/utf-8")
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
endif()
