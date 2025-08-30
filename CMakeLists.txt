cmake_minimum_required(VERSION 3.20)

project(FlecsInnerDataVisualizer)

add_subdirectory(glfw)
add_subdirectory(imgui)
add_subdirectory(glad)
add_subdirectory(flecs)

add_subdirectory(src)