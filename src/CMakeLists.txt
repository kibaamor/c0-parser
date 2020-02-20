file(GLOB SRC_LIST 
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)

add_library(${CMAKE_PROJECT_NAME} STATIC ${SRC_LIST})
source_group(tokenizer REGULAR_EXPRESSION ".*tokenizer.(h|cpp)$")
source_group(analyser REGULAR_EXPRESSION ".*analyser.(h|cpp)$")
source_group(ast REGULAR_EXPRESSION ".*ast.(h|cpp)$")

target_include_directories(${CMAKE_PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})