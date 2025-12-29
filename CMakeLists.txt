cmake_minimum_required(VERSION 4.1.2)
project(chess)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)

include_directories(include src thirdparty)

file(GLOB IMGUI_SOURCES
    thirdparty/imgui/*.cpp
)

add_executable(chess
    #src/main.cpp
    src/main_debug.cpp
    src/stb_impl.cpp
    include/board.cpp
    include/move_generator.cpp
    thirdparty/glad.c
    ${IMGUI_SOURCES}
)

target_link_libraries(chess
    glfw
    OpenGL::GL
)

add_definitions(-Wno-deprecated-declarations)
