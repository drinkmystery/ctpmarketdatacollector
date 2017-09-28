file(GLOB_RECURSE CPP_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.cpp)
file(GLOB_RECURSE HEADER_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.h)

add_executable(ctp-marketdata-collector ${CPP_FILES} ${HEADER_FILES})
target_compile_options(ctp-marketdata-collector PRIVATE "/utf-8")
