add_executable(StrToNum NumTest.cpp)
# target_link_libraries(INPUtilTest Application Camera GeomMath Physics Utility Framework)
target_link_libraries(StrToNum)

set_target_properties(StrToNum PROPERTIES FOLDER "Test")