cmake_minimum_required(VERSION 3.23)
project(MyProjectName C)

set(CMAKE_C_STANDARD 11)

find_package(CURL REQUIRED)

add_executable(fide_tui
    src/main.c
    src/data.c
)


target_link_libraries(fide_tui PRIVATE curl minizip)

