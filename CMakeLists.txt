cmake_minimum_required(VERSION 3.30)

set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "0e5b6991-d74f-4b3d-a41c-cf096e0b2508")
set(CMAKE_CXX_MODULE_STD ON)

project(opengl_renderer LANGUAGES CXX C)

set(CMAKE_EXPERIMENTAL_CXX_MODULE_DYNDEP 1)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# set(CMAKE_CXX_EXTENSIONS OFF)

include(cmake/CPM.cmake)

CPMAddPackage(
    NAME glfw
    GITHUB_REPOSITORY glfw/glfw
    GIT_TAG 3.3.9
    OPTIONS
        "GLFW_BUILD_EXAMPLES OFF"
        "GLFW_BUILD_TESTS OFF"
        "GLFW_BUILD_DOCS OFF"
        "GLFW_INSTALL OFF"
)

CPMAddPackage(
    NAME glm
    GITHUB_REPOSITORY g-truc/glm
    GIT_TAG 1.0.1
    OPTIONS
    "GLM_TEST_ENABLE OFF"
)

CPMAddPackage(
    NAME stb
    GITHUB_REPOSITORY nothings/stb
    GIT_TAG master
)

add_library(glad STATIC external/glad/src/gl.c)
target_include_directories(glad PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/external/glad/include)
target_compile_features(glad PUBLIC c_std_99)

add_library(opengl_renderer STATIC)

target_sources(opengl_renderer
    PUBLIC
    FILE_SET opengl_renderer_modules TYPE CXX_MODULES FILES
        src/core/app.cppm
        src/scenes/hello_window/scene.cppm
        src/scenes/hello_triangle/scene.cppm
        src/render/buffer.cppm
        src/render/vertex_array.cppm
        src/render/shader.cppm
        src/gpu/gl.cppm
        src/math/glm.cppm
        src/platform/glfw.cppm
        src/scenes/hello_square/scene.cppm
        src/resources/image.cppm
)

target_compile_options(opengl_renderer PUBLIC
    -stdlib=libc++
    -Wno-module-import-in-extern-c
)

target_link_libraries(opengl_renderer PUBLIC
    glad
    glfw
    glm::glm
)

target_include_directories(opengl_renderer PUBLIC
    ${stb_SOURCE_DIR}
)

add_executable(opengl_renderer_executable src/main.cpp)
target_link_libraries(opengl_renderer_executable PRIVATE opengl_renderer)
target_compile_options(opengl_renderer_executable PRIVATE -stdlib=libc++)
