add_executable(ShadowMapping main.cpp)
target_link_libraries(ShadowMapping Application Camera GeomMath Physics Utility Framework)

set_target_properties(ShadowMapping PROPERTIES FOLDER "Test")