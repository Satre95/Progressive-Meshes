link_libraries(StarForge glfw)

include_directories(${glfw_INCLUDE_DIRS} "${GLFW_SOURCE_DIR}/deps" "../externals/assimp/include")

if(MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

set(GLAD "${GLFW_SOURCE_DIR}/deps/glad/glad.h"
         "${GLFW_SOURCE_DIR}/deps/glad.c")

set(SOURCE_FILES
    main.cpp
    ProgModel.cpp
    ProgMesh.cpp
    Decimation.cpp)
set(HEADER_FILES
    ProgModel.hpp
    ProgMesh.hpp
    Geometry.hpp
    Decimation.hpp)

add_executable(ProgressiveMeshes ${SOURCE_FILES} ${HEADER_FILES} ${GLAD})

if(WIN32)
	if(MSVC) # Check if we are using the Visual Studio compiler
        set_target_properties(ProgressiveMeshes PROPERTIES LINK_FLAGS "/SUBSYSTEM:CONSOLE")
	endif()
endif()
set(WINDOWS_BINARIES ProgressiveMeshes)

set_target_properties(${WINDOWS_BINARIES} PROPERTIES FOLDER "Progressive Meshes")
