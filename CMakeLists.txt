cmake_minimum_required(VERSION 3.8)
project(StarForge)

set(CMAKE_CXX_STANDARD 14) # C++14...
set(CMAKE_CXX_STANDARD_REQUIRED ON) #...is required...
set(CMAKE_CXX_EXTENSIONS OFF) #...without compiler extensions like gnu++11
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(THREADS_PREFER_PTHREAD ON)
set(CMAKE_MACOSX_RPATH 1)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

OPTION (ENABLE_OpenMP "Enable OpenMP" ON)

if(ENABLE_OpenMP)
    find_package(OpenMP)
    if(OPENMP_FOUND)
        message("Compiling with OpenMP support")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    endif()
endif()

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

if(MSVC)
    add_compile_options(/MP)
endif()

# Add external libs
add_subdirectory(externals)

# Build StarForge
add_subdirectory(source)

# Build examples
add_subdirectory(examples)
