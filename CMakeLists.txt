cmake_minimum_required(VERSION 3.8)

project(ctp-marketdata-collector)

find_package(Boost REQUIRED)
find_package(libmongocxx REQUIRED)

include_directories(third/include)
link_directories(third/prebuilt/win64)

add_subdirectory(src)
