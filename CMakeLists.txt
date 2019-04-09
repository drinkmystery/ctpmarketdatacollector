cmake_minimum_required(VERSION 3.8)

project(ctp-marketdata-collector)

find_package(Boost REQUIRED COMPONENTS
             program_options filesystem system)
message(${Boost_INCLUDE_DIRS})
message(${Boost_LIBRARIES})

find_package(libmongoc-static-1.0 REQUIRED)
# message(${MONGOC_STATIC_INCLUDE_DIRS})
# message(${MONGOC_STATIC_LIBRARIES})
# message(${MONGOC_STATIC_DEFINITIONS})

find_package(libmongocxx REQUIRED)
message(${LIBMONGOCXX_INCLUDE_DIRS})
message(${LIBMONGOCXX_LIBRARY_DIRS})
message(${LIBMONGOCXX_LIBRARIES})

find_package(libbsoncxx REQUIRED)
# message(${LIBBSONCXX_INCLUDE_DIRS})
# message(${LIBBSONCXX_LIBRARY_DIRS})
# message(${LIBBSONCXX_LIBRARIES})

include_directories(${MONGOC_STATIC_INCLUDE_DIRS})
include_directories(${Boost_INCLUDE_DIRS})
include_directories(${LIBMONGOCXX_INCLUDE_DIRS})
include_directories(${LIBBSONCXX_INCLUDE_DIRS})
include_directories(third/include)

link_directories(${LIBMONGOCXX_LIBRARY_DIRS})
link_directories(${LIBBSONCXX_LIBRARY_DIRS})
link_directories(third/prebuilt/Win64)

set(NLOHMANN_JSON_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third/include/json)
message(${NLOHMANN_JSON_DIR})
add_subdirectory(third/cmake/json-schema-validator)

add_subdirectory(src)
