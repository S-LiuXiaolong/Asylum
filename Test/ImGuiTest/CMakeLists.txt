add_executable(ImGuiTest main.cpp)
target_link_libraries(ImGuiTest Application Camera GeomMath xxHash Physics Utility Framework)

find_library(IMGUI_LIBRARY_DEBUG       imgui PATHS ${MYGE_EXTERNAL_LIBRARY_PATH}/Debug)
find_library(IMGUI_LIBRARY_RELEASE     imgui PATHS ${MYGE_EXTERNAL_LIBRARY_PATH}/Release)

target_link_libraries(ImGuiTest 
            debug ${IMGUI_LIBRARY_DEBUG}
            )
target_link_libraries(ImGuiTest 
            optimized ${IMGUI_LIBRARY_RELEASE}
            )

set_target_properties(ImGuiTest PROPERTIES FOLDER "Test")