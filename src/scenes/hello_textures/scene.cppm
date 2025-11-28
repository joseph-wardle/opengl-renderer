export module scenes.hello_textures;

import std;
import render.context;
import gpu.gl;
import core.app;
import platform.glfw;
import render.vertex_array;
import render.buffer;
import render.shader;
import render.texture;
import resources.image;

export namespace scenes {

struct HelloTextured {
    explicit HelloTextured(bool wireframe = false) : wireframe_(wireframe) {}
    render::Context ctx{};

    void on_init() {
        constexpr std::array<float, 20> vertices{
           // positions        // texcoords
           -0.5f, -0.5f, 0.0f,  0.0f, 0.0f,
            0.5f, -0.5f, 0.0f,  1.0f, 0.0f,
            0.5f,  0.5f, 0.0f,  1.0f, 1.0f,
           -0.5f,  0.5f, 0.0f,  0.0f, 1.0f,
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
            static_cast<int>(5 * sizeof(float)), // stride bytes
            0 // offset bytes
        );
        vao_.set_attribute_float(
            1, // index
            2, // component count
            static_cast<int>(5 * sizeof(float)), // stride bytes
            3 * sizeof(float) // offset bytes
        );

        render::VertexArray::unbind();
        render::VertexBuffer::unbind();
        render::IndexBuffer::unbind();

        const auto shader_dir = std::filesystem::path(__FILE__).parent_path();
        auto shader_result = render::Shader::from_files(
            shader_dir / "textured.vert",
            shader_dir / "textured.frag"
        );
        if (!shader_result) {
            std::println(std::cerr, "Failed to create shader for HelloTextured: {}", shader_result.error());
            return;
        }
        shader_ = std::move(*shader_result);

        auto tex_path_a = shader_dir / "texture1.png";
        auto tex_path_b = shader_dir / "texture2.png";
        auto img_a = resources::load_image(tex_path_a.string());
        auto img_b = resources::load_image(tex_path_b.string());
        if (!img_a) {
            std::println(std::cerr, "Failed to load texture A ({}): {}", tex_path_a.string(), img_a.error());
            return;
        }
        if (!img_b) {
            std::println(std::cerr, "Failed to load texture B ({}): {}", tex_path_b.string(), img_b.error());
            return;
        }

        auto tex_a = render::Texture2D::from_image(*img_a, true);
        if (!tex_a) {
            std::println(std::cerr, "Failed to create texture A: {}", tex_a.error());
            return;
        }
        auto tex_b = render::Texture2D::from_image(*img_b, true);
        if (!tex_b) {
            std::println(std::cerr, "Failed to create texture B: {}", tex_b.error());
            return;
        }

        texture_a_ = std::move(*tex_a);
        texture_b_ = std::move(*tex_b);
        std::println("Loaded textures: A id {} ({}x{}), B id {} ({}x{})",
                     texture_a_.id(), img_a->width, img_a->height,
                     texture_b_.id(), img_b->width, img_b->height);

        ctx.set_wireframe(wireframe_);
    }

    void on_update(core::DeltaTime dt, const platform::InputState&, bool) {
        time_ += dt.seconds;
        blend_ = 0.5f + 0.5f * std::sin(time_);
    }

    void on_render() {
        ctx.begin_frame(render::FrameClear{0.1f, 0.1f, 0.1f, 1.0f});

        if (!vao_.is_valid() || !ibo_.is_valid() || !texture_a_.is_valid() || !texture_b_.is_valid()) {
            return;
        }
        if (shader_.id() == 0) {
            std::println(std::cerr, "Shader ID is 0!");
            return;
        }

        shader_.use();
        shader_.set_float("uBlend", blend_);
        shader_.set_int("uTex0", 0);
        shader_.set_int("uTex1", 1);
        texture_a_.bind(0);
        texture_b_.bind(1);
        vao_.bind();
        ibo_.bind();
        gpu::gl::draw_elements(gpu::gl::Primitive::triangles, 6, gpu::gl::IndexType::u32);
        render::IndexBuffer::unbind();
        render::VertexArray::unbind();
    }

    void on_gui() {}

    void on_resize(int width, int height) {
        ctx.set_viewport(width, height);
    }
private:
    bool                 wireframe_{false};
    float                time_{0.0f};
    float                blend_{0.5f};
    render::VertexArray  vao_{};
    render::VertexBuffer vbo_{};
    render::IndexBuffer  ibo_{};
    render::Texture2D    texture_a_{};
    render::Texture2D    texture_b_{};
    render::Shader       shader_{};
};

}
