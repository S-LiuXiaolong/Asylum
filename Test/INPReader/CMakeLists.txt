add_executable(INPReader main.cpp)
target_link_libraries(INPReader Application Camera GeomMath xxHash Physics Utility Framework)

set_target_properties(INPReader PROPERTIES FOLDER "Test")