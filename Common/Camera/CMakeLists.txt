add_library(Camera
    basiccamera.cpp
    fpscamera.cpp
    spectatorcamera.cpp
)

set_target_properties(Camera PROPERTIES FOLDER "Common")