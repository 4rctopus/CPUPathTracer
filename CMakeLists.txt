cmake_minimum_required(VERSION 3.17)
project(Pathtracer)


set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -fopenmp")

add_executable(Pathtracer main.cpp imgui/imgui.cpp imgui/imgui_draw.cpp
        imgui/imgui_demo.cpp imgui/imgui_tables.cpp imgui/imgui_widgets.cpp
        imgui/imgui_impl_opengl3.cpp imgui/imgui_impl_sdl.cpp Material.h Ray.h AABB.h BVHnode.h PDF.h)

target_link_libraries(Pathtracer mingw32 glew32 opengl32 SDL2main SDL2 imm32 )