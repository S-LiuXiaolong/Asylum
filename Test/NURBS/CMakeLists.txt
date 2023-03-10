add_executable(NURBS main.cpp)
target_link_libraries(NURBS Application Camera GeomMath Physics Utility Framework)

set_target_properties(NURBS PROPERTIES FOLDER "Test")