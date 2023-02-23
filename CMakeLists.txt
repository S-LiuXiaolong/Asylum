cmake_minimum_required(VERSION 3.6)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 11)

project(Asylum)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

include_directories("${PROJECT_SOURCE_DIR}/Common/Application")
include_directories("${PROJECT_SOURCE_DIR}/Common/Camera")
include_directories("${PROJECT_SOURCE_DIR}/Common/GeomMath")
include_directories("${PROJECT_SOURCE_DIR}/Common/Hash")
include_directories("${PROJECT_SOURCE_DIR}/Common/Physics")
include_directories("${PROJECT_SOURCE_DIR}/Common/Utility")

include_directories("${PROJECT_SOURCE_DIR}/include/OpenGL")

IF(${WIN32})
    add_definitions(/D _CRT_SECURE_NO_WARNINGS)
    set(ASYLUM_TARGET_PLATFORM "Windows")
ENDIF(${WIN32})

add_definitions(/D OPENGL)

add_subdirectory(Common)
add_subdirectory(Framework)
add_subdirectory(Test)