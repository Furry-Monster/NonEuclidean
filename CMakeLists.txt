cmake_minimum_required(VERSION 3.10)
project(NonEuclidean)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenGL REQUIRED)

find_package(glfw3 QUIET)
if(NOT glfw3_FOUND)
    find_package(PkgConfig QUIET)
    if(PkgConfig_FOUND)
        pkg_check_modules(GLFW3 QUIET glfw3)
    endif()
    if(NOT GLFW3_FOUND AND NOT glfw3_FOUND)
        message(FATAL_ERROR "GLFW not found. Please install libglfw3-dev (Ubuntu/Debian) or glfw (Arch)")
    endif()
endif()

find_package(GLEW QUIET)
if(NOT GLEW_FOUND)
    find_package(PkgConfig QUIET)
    if(PkgConfig_FOUND)
        pkg_check_modules(GLEW QUIET glew)
    endif()
    if(NOT GLEW_FOUND)
        message(FATAL_ERROR "GLEW not found. Please install libglew-dev (Ubuntu/Debian) or glew (Arch)")
    endif()
endif()

set(SOURCES
    NonEuclidean/Main.cpp
    NonEuclidean/Camera.cpp
    NonEuclidean/Collider.cpp
    NonEuclidean/Engine.cpp
    NonEuclidean/FrameBuffer.cpp
    NonEuclidean/Input.cpp
    NonEuclidean/Level1.cpp
    NonEuclidean/Level2.cpp
    NonEuclidean/Level3.cpp
    NonEuclidean/Level4.cpp
    NonEuclidean/Level5.cpp
    NonEuclidean/Level6.cpp
    NonEuclidean/Mesh.cpp
    NonEuclidean/Object.cpp
    NonEuclidean/Physical.cpp
    NonEuclidean/Player.cpp
    NonEuclidean/Portal.cpp
    NonEuclidean/Resources.cpp
    NonEuclidean/Shader.cpp
    NonEuclidean/Texture.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/NonEuclidean
)

if(glfw3_FOUND)
    target_link_libraries(${PROJECT_NAME} glfw)
elseif(GLFW3_FOUND)
    target_include_directories(${PROJECT_NAME} PRIVATE ${GLFW3_INCLUDE_DIRS})
    target_link_libraries(${PROJECT_NAME} ${GLFW3_LIBRARIES})
endif()

if(GLEW_FOUND)
    target_include_directories(${PROJECT_NAME} PRIVATE ${GLEW_INCLUDE_DIRS})
    target_link_libraries(${PROJECT_NAME} ${GLEW_LIBRARIES})
endif()

target_link_libraries(${PROJECT_NAME} ${OPENGL_LIBRARIES})

if(UNIX AND NOT APPLE)
    target_link_libraries(${PROJECT_NAME} X11)
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)

file(COPY NonEuclidean/Shaders DESTINATION ${CMAKE_BINARY_DIR})
file(COPY NonEuclidean/Textures DESTINATION ${CMAKE_BINARY_DIR})
file(COPY NonEuclidean/Meshes DESTINATION ${CMAKE_BINARY_DIR})
