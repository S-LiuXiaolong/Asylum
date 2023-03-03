add_executable(glNURBS main.cpp)
target_link_libraries(glNURBS Application Camera GeomMath xxHash Physics Utility Framework)

set_target_properties(glNURBS PROPERTIES FOLDER "Test")