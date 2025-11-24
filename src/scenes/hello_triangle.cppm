export module scenes.hello_triangle;

import std;
import gpu.gl;
import core.app;
import platform.glfw;
import render.vertex_array;
import render.buffer;
import render.shader;

export namespace scenes {

inline constexpr char kVertexSource[] = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;

out vec3 vColor;

void main()
{
    gl_Position = vec4(aPos, 1.0);
    vColor = aColor;
}
)";

inline constexpr char kFragmentSource[] = R"(
#version 330 core
in vec3 vColor;
out vec4 FragColor;

void main()
{
    FragColor = vec4(vColor, 1.0);
}
)";
    
struct HelloTriangle {
    void on_init() {
        constexpr std::array<float, 18> vertices{
           // position           // color
           -0.5f, -0.5f, 0.0f,    1.0f, 0.0f, 0.0f,
            0.5f, -0.5f, 0.0f,    0.0f, 1.0f, 0.0f,
            0.0f,  0.5f, 0.0f,    0.0f, 0.0f, 1.0f,
        };
        
        auto vbo_result = render::VertexBuffer::from_data(
            vertices.data(),
            vertices.size() * sizeof(float)
        );
        if (!vbo_result) {
            std::println(std::cerr, "VBO creation failed: {}", vbo_result.error());
            return;
        }
        vbo_ = std::move(*vbo_result);

        auto vao_result = render::VertexArray::create();
        if (!vao_result) {
            std::println(std::cerr, "VAO creation failed: {}", vao_result.error());
            return;
        }
        vao_ = std::move(*vao_result);
        
        vao_.bind();
        vbo_.bind();
        
        vao_.set_attribute_float(
            0, // index
            3, // component count
            static_cast<int>(6 * sizeof(float)), // stride bytes
            0 // offset bytes
        );
        vao_.set_attribute_float(
            1, // index
            3, // component count
            static_cast<int>(6 * sizeof(float)), // stride bytes
            3 * sizeof(float) // offset bytes
        );
        
        render::VertexArray::unbind();
        render::VertexBuffer::unbind();
        
        auto shader_result = render::Shader::from_source(kVertexSource, kFragmentSource);
        if (!shader_result) {
            std::println(std::cerr, "Failed to create shader for HelloTriangle: {}", shader_result.error());
            return;
        }
        shader_ = std::move(*shader_result);
    }
    
    void on_update(core::DeltaTime, const platform::InputState&) {
        // handle input later
    }

    void on_render() {
        gpu::gl::clear_color(0.2f, 0.3f, 0.3f, 1.0f);
        gpu::gl::clear(gpu::gl::COLOR_BUFFER_BIT);
        
        if (!vao_.is_valid()) {
            return;
        }
        if (shader_.id() == 0) {
            return;
        }

        shader_.use();
        vao_.bind();
        gpu::gl::draw_arrays(gpu::gl::Primitive::triangles, 0, 3);
        render::VertexArray::unbind();
    }

    void on_resize(int width, int height) {
        gpu::gl::viewport(0, 0, width, height);
    }
private:
    render::VertexArray  vao_{};
    render::VertexBuffer vbo_{};
    render::Shader       shader_{};
};

}
