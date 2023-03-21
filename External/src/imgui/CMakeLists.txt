cmake_minimum_required(VERSION 3.6)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 11)

project(imgui)

include_directories(.)

add_library(imgui)
target_sources(imgui PRIVATE
    imgui.cpp
)

set(MYGE_EXTERNAL_ROOT ${PROJECT_BINARY_DIR}/../..)

set_target_properties(imgui
PROPERTIES PUBLIC_HEADER imgui.h
)

install(TARGETS imgui
    PUBLIC_HEADER DESTINATION ${MYGE_EXTERNAL_ROOT}/include/imgui
    ARCHIVE DESTINATION ${MYGE_EXTERNAL_ROOT}/lib/Debug
    CONFIGURATIONS Debug
)

install(TARGETS imgui
    ARCHIVE DESTINATION ${MYGE_EXTERNAL_ROOT}/lib/Release
    CONFIGURATIONS Release
)

if(MSVC)
    install(FILES ${PROJECT_BINARY_DIR}/Debug/imgui.pdb
        DESTINATION ${MYGE_EXTERNAL_ROOT}/lib/Debug
        CONFIGURATIONS Debug
        )
endif(MSVC)