add_executable(Ocean main.cpp fft_test.cpp)
target_link_libraries(Ocean Application Camera GeomMath Physics Utility Framework)

set_target_properties(Ocean PROPERTIES FOLDER "Test")