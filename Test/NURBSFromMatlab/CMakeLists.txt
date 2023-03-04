add_executable(NURBSFromMatlab main.cpp)
target_link_libraries(NURBSFromMatlab Application Camera GeomMath Physics Utility Framework)

set_target_properties(NURBSFromMatlab PROPERTIES FOLDER "Test")