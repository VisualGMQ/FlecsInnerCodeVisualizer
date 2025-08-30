file(GLOB_RECURSE SRC *.hpp *.cpp)

add_executable(main)
target_sources(main PRIVATE ${SRC})
target_link_libraries(main PRIVATE glfw imgui glad flecs::flecs_static)