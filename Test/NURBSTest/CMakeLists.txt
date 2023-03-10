add_executable(NURBSTest main.cpp)
target_link_libraries(NURBSTest Application Camera GeomMath Physics Utility Framework)

set_target_properties(NURBSTest PROPERTIES FOLDER "Test")