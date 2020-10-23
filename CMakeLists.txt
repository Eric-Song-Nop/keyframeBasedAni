cmake_minimum_required(VERSION 2.8.12)
project(testOpenGL)

add_definitions("-std=c++11")

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

add_executable(testOpenGL testOpenGL.cpp camera.h Mesh.hpp Shader.hpp Model.hpp Shader_m.hpp imgui/imgui_impl_glfw.cpp imgui/imgui_impl_glfw.h imgui/imgui_impl_opengl3.cpp imgui/imgui_impl_opengl3.h cubicSpliner.h )
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
target_link_libraries(testOpenGL ${CONAN_LIBS})
