# compile imgui
file(GLOB IMGUI_SRC imgui/*.h imgui/*.cpp)
set(IMGUI_BACKEND
    imgui/backends/imgui_impl_glfw.h
    imgui/backends/imgui_impl_glfw.cpp
    imgui/backends/imgui_impl_opengl3.h
    imgui/backends/imgui_impl_opengl3.cpp
    imgui/backends/imgui_impl_opengl3_loader.h)
add_library(imgui STATIC)
target_sources(imgui PRIVATE ${IMGUI_SRC} ${IMGUI_BACKEND})
target_include_directories(imgui PUBLIC imgui)
target_link_libraries(imgui PRIVATE glfw)