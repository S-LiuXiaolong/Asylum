add_executable(INPUtilTest main.cpp)
# target_link_libraries(INPUtilTest Application Camera GeomMath Physics Utility Framework)
target_link_libraries(INPUtilTest GeomMath)

set_target_properties(INPUtilTest PROPERTIES FOLDER "Test")

add_subdirectory(StrToNum)