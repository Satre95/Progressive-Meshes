include_directories("${GLFW_SOURCE_DIR}/deps" "../externals/assimp/include")
include_directories("${GLFW_SOURCE_DIR}/deps" "../build/externals/assimp/include")

set(HEADER_FILES
    ../include/RenderDevice.hpp ../include/OpenGLRenderDevice.hpp
    ../include/Platform.hpp ../include/Model.hpp
    ../include/Mesh.hpp ../include/DefaultShaders.hpp
    ../include/Node.hpp ../include/GroupNode.hpp
    ../include/MatrixTransformNode.hpp ../include/Geode.hpp
    ../include/SceneGraph.hpp
    ../include/Cube.hpp
    ../include/OpenGLDepthRasterStates.hpp
    ../include/OpenGLPipeline.hpp ../include/Utilities.hpp
    )

set(SOURCE_FILES
    RenderDevice.cpp OpenGLRenderDevice.cpp
    GLFW_Platform.cpp
    Model.cpp Mesh.cpp
    Cube.cpp DefaultShaders.cpp Node.cpp
    GroupNode.cpp MatrixTransformNode.cpp
    Geode.cpp SceneGraph.cpp
    )


add_library(StarForge STATIC ${HEADER_FILES} ${SOURCE_FILES} )

target_link_libraries(StarForge glfw glm assimp)

target_include_directories(StarForge PUBLIC ../include)
