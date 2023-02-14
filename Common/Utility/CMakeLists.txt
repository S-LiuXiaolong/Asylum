add_library(Utility
    dds.cpp
    geometryutils.cpp
    gl4ext.cpp
    glextensions.cpp
    lightning.cpp
    particlesystem.cpp
    terrainquadtree.cpp
)

set_target_properties(Utility PROPERTIES FOLDER "Common")