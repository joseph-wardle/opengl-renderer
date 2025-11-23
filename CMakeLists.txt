cmake_minimum_required(VERSION 3.30)

set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD
    "0e5b6991-d74f-4b3d-a41c-cf096e0b2508")
set(CMAKE_CXX_MODULE_STD ON)

project(opengl_renderer LANGUAGES CXX C)

set(CMAKE_EXPERIMENTAL_CXX_MODULE_DYNDEP 1)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(opengl_renderer STATIC)

target_sources(opengl_renderer
    PUBLIC
    FILE_SET opengl_renderer_modules TYPE CXX_MODULES FILES
        src/core/app.cppm
)

target_compile_options(opengl_renderer PUBLIC
    -stdlib=libc++
)

add_executable(opengl_renderer_executable src/main.cpp)
target_link_libraries(opengl_renderer_executable PRIVATE opengl_renderer)
target_compile_options(opengl_renderer_executable PRIVATE -stdlib=libc++)
