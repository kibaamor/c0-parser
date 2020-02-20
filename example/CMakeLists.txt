add_executable(dump_token dump_token.cpp)
target_link_libraries(dump_token ${CMAKE_PROJECT_NAME})
set_property(TARGET dump_token PROPERTY FOLDER "example")

add_executable(rewrite_ast rewrite_ast.cpp)
target_link_libraries(rewrite_ast ${CMAKE_PROJECT_NAME})
set_property(TARGET rewrite_ast PROPERTY FOLDER "example")