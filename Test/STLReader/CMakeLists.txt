add_executable(STLReader main.cpp)
target_link_libraries(STLReader Application Camera GeomMath Physics Utility Framework)

set_target_properties(STLReader PROPERTIES FOLDER "Test")