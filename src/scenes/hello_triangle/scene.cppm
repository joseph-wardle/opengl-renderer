export module scenes.hello_triangle;

import std;
import render.context;
import gpu.gl;
import core.app;
import platform.glfw;
import render.vertex_array;
import render.buffer;
import render.shader;

export namespace scenes {

struct HelloTriangle {
    render::Context ctx{};

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

        const auto shader_dir = std::filesystem::path(__FILE__).parent_path();
        auto shader_result = render::Shader::from_files(
            shader_dir / "triangle.vert",
            shader_dir / "triangle.frag"
        );
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
        ctx.begin_frame(render::FrameClear{0.2f, 0.3f, 0.3f, 1.0f});
        
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

    void on_gui() {}

    void on_resize(int width, int height) {
        ctx.set_viewport(width, height);
    }
private:
    render::VertexArray  vao_{};
    render::VertexBuffer vbo_{};
    render::Shader       shader_{};
};

}
