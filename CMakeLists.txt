cmake_minimum_required(VERSION 3.0.0)
project(SemestralProject)


set(CMAKE_CXX_STANDARD 23)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
#add_compile_options(-Wall -fdiagnostics-color=always)

add_executable(SemestralProject main.cpp)

include(FetchContent)
FetchContent_Declare(
        ds
        GIT_REPOSITORY https://github.com/DrLubos/DataStructures.git
        GIT_TAG main
)
FetchContent_MakeAvailable(ds)

target_link_libraries(SemestralProject PRIVATE ds)

