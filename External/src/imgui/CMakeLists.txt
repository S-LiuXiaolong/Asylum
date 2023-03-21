cmake_minimum_required(VERSION 3.6)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 11)

project(imgui)

include_directories(.)

add_library(imgui)
target_sources(imgui PRIVATE
    imgui_demo.cpp
    imgui_draw.cpp
    imgui_tables.cpp
    imgui_widgets.cpp
    imgui.cpp
    backends/imgui_impl_opengl3.cpp
    backends/imgui_impl_win32.cpp
)

set(MYGE_EXTERNAL_ROOT ${PROJECT_BINARY_DIR}/../..)

set(HEADER_FILES
    imgui.h
    imconfig.h
    backends/imgui_impl_opengl3.h
    backends/imgui_impl_win32.h
)

set_target_properties(imgui PROPERTIES  
    PUBLIC_HEADER "${HEADER_FILES}"
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