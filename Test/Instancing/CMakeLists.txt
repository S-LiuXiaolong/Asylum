add_executable(Instancing main.cpp)
target_link_libraries(Instancing Application Camera GeomMath Physics Utility Framework)

set_target_properties(Instancing PROPERTIES FOLDER "Test")