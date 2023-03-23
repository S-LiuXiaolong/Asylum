add_executable(SphereInstancing main.cpp)
target_link_libraries(SphereInstancing Application Camera GeomMath Physics Utility Framework)

set_target_properties(SphereInstancing PROPERTIES FOLDER "Test")