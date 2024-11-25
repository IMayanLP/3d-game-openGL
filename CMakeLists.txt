cmake_minimum_required(VERSION 3.25)
project(3d_game_openGL C)

set(CMAKE_C_STANDARD 11)

include_directories(C:/MinGW/include)
link_directories(C:/MinGW/lib)

add_executable(3d_game_openGL
        inimigos.c
        inimigos.h
        main.c)

target_link_libraries(3d_game_openGL opengl32 glu32 freeglut)