export module scenes.hello_square;

import std;
import gpu.gl;
import core.app;
import platform.glfw;
import render.vertex_array;
import render.buffer;
import render.shader;

export namespace scenes {

inline constexpr char kVertexSourceSquare[] = R"(
#version 330 core
layout (location = 0) in vec3 aPos;

void main()
{
    gl_Position = vec4(aPos, 1.0);
}
)";

inline constexpr char kFragmentSourceSquare[] = R"(
#version 330 core
out vec4 FragColor;
uniform float uTime;

void main()
{
    float r = 0.5 + 0.5 * sin(uTime * 0.8);
    float g = 0.5 + 0.5 * sin(uTime * 1.3 + 2.0);
    float b = 0.5 + 0.5 * sin(uTime * 1.7 + 4.0);
    FragColor = vec4(r, g, b, 1.0);
}
)";
    
struct HelloSquare {
    explicit HelloSquare(bool wireframe = false) : wireframe_(wireframe) {}

    void on_init() {
        constexpr std::array<float, 12> vertices{
           -0.5f, -0.5f, 0.0f,
            0.5f, -0.5f, 0.0f,
            0.5f,  0.5f, 0.0f,
           -0.5f,  0.5f, 0.0f,
        };

        constexpr std::array<unsigned int, 6> indices{
            0, 1, 2,
            0, 2, 3
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

        auto ibo_result = render::IndexBuffer::from_data(
            indices.data(),
            indices.size() * sizeof(unsigned int)
        );
        if (!ibo_result) {
            std::println(std::cerr, "EBO creation failed: {}", ibo_result.error());
            return;
        }
        ibo_ = std::move(*ibo_result);

        auto vao_result = render::VertexArray::create();
        if (!vao_result) {
            std::println(std::cerr, "VAO creation failed: {}", vao_result.error());
            return;
        }
        vao_ = std::move(*vao_result);
        
        vao_.bind();
        vbo_.bind();
        ibo_.bind();
        
        vao_.set_attribute_float(
            0, // index
            3, // component count
            static_cast<int>(3 * sizeof(float)), // stride bytes
            0 // offset bytes
        );
        
        render::VertexArray::unbind();
        render::VertexBuffer::unbind();
        render::IndexBuffer::unbind();
        
        auto shader_result = render::Shader::from_source(kVertexSourceSquare, kFragmentSourceSquare);
        if (!shader_result) {
            std::println(std::cerr, "Failed to create shader for HelloSquare: {}", shader_result.error());
            return;
        }
        shader_ = std::move(*shader_result);

        gpu::gl::polygon_mode(
            gpu::gl::Face::front_and_back,
            wireframe_ ? gpu::gl::PolygonMode::line : gpu::gl::PolygonMode::fill
        );
    }
    
    void on_update(core::DeltaTime dt, const platform::InputState&) {
        time_ += dt.seconds;
    }

    void on_render() {
        gpu::gl::clear_color(0.1f, 0.1f, 0.1f, 1.0f);
        gpu::gl::clear(gpu::gl::COLOR_BUFFER_BIT);
        
        if (!vao_.is_valid() || !ibo_.is_valid()) {
            return;
        }
        if (shader_.id() == 0) {
            return;
        }

        shader_.use();
        if (auto loc = gpu::gl::get_uniform_location(shader_.id(), "uTime"); loc != -1) {
            gpu::gl::set_uniform(loc, time_);
        }
        vao_.bind();
        ibo_.bind();
        gpu::gl::draw_elements(gpu::gl::Primitive::triangles, 6, gpu::gl::IndexType::u32);
        render::IndexBuffer::unbind();
        render::VertexArray::unbind();
    }

    void on_resize(int width, int height) {
        gpu::gl::viewport(0, 0, width, height);
    }
private:
    bool                 wireframe_{false};
    float                time_{0.0f};
    render::VertexArray  vao_{};
    render::VertexBuffer vbo_{};
    render::IndexBuffer  ibo_{};
    render::Shader       shader_{};
};

}
