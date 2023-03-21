add_library(Application
    application.cpp
    win32application.cpp
)

# find_library(IMGUI_LIBRARY_DEBUG       imgui PATHS ${MYGE_EXTERNAL_LIBRARY_PATH}/Debug)
# find_library(IMGUI_LIBRARY_RELEASE     imgui PATHS ${MYGE_EXTERNAL_LIBRARY_PATH}/Release)

# target_link_libraries(Application 
#             debug ${IMGUI_LIBRARY_DEBUG}
#             )
# target_link_libraries(Application 
#             optimized ${IMGUI_LIBRARY_RELEASE}
#             )

set_target_properties(Application PROPERTIES FOLDER "Common")