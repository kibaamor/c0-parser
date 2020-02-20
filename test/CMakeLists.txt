add_executable(tokenizer tokenizer.cpp doctest.h)
target_link_libraries(tokenizer ${CMAKE_PROJECT_NAME})
set_property(TARGET tokenizer PROPERTY FOLDER "test")
add_test(NAME test_tokenizer COMMAND $<TARGET_FILE:tokenizer>)