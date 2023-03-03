add_executable(teapot main.cpp)
target_link_libraries(teapot Application Camera GeomMath Physics Utility Framework)

set_target_properties(teapot PROPERTIES FOLDER "Test")