file(GLOB_RECURSE SRC *.c *.h)
add_library(glad STATIC)
target_sources(glad PRIVATE ${SRC})
target_include_directories(glad PUBLIC include)