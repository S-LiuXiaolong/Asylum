add_executable(NURBSTest main.cpp)
target_link_libraries(NURBSTest Application Camera GeomMath Physics Utility Framework)

find_library(IMGUI_LIBRARY_DEBUG       imgui PATHS ${MYGE_EXTERNAL_LIBRARY_PATH}/Debug)
find_library(IMGUI_LIBRARY_RELEASE     imgui PATHS ${MYGE_EXTERNAL_LIBRARY_PATH}/Release)

target_link_libraries(NURBSTest 
            debug ${IMGUI_LIBRARY_DEBUG}
            )
target_link_libraries(NURBSTest 
            optimized ${IMGUI_LIBRARY_RELEASE}
            )

set_target_properties(NURBSTest PROPERTIES FOLDER "Test")