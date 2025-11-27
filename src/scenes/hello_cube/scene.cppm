export module scenes.hello_cube;

import std;
import render.context;
import gpu.gl;
import core.app;
import core.glm;
import platform.glfw;
import render.vertex_array;
import render.buffer;
import render.shader;
import render.texture;
import resources.image;

export namespace scenes {

struct HelloCube {
    HelloCube() {
        std::mt19937 rng{std::random_device{}()};
        std::uniform_real_distribution<float> dist{-1.0f, 1.0f};
        // Randomize rotation axis and speed a bit for variety.
        rotation_axis_ = core::Vec3{dist(rng), dist(rng), dist(rng)};
        if (length(rotation_axis_) < 0.001f) {
            rotation_axis_ = core::Vec3{0.0f, 1.0f, 0.0f};
        } else {
            rotation_axis_ = normalize(rotation_axis_);
        }
        rotation_speed_ = 0.5f + std::abs(dist(rng)) * 0.5f; // radians/sec
        bounce_speed_   = 1.0f + std::abs(dist(rng)) * 0.5f;
    }

    void on_init() {
        constexpr std::array<float, 180> vertices = {
            // positions           // uvs
            // front
            -0.5f, -0.5f,  0.5f,    0.0f, 0.0f,
             0.5f, -0.5f,  0.5f,    1.0f, 0.0f,
             0.5f,  0.5f,  0.5f,    1.0f, 1.0f,
             0.5f,  0.5f,  0.5f,    1.0f, 1.0f,
            -0.5f,  0.5f,  0.5f,    0.0f, 1.0f,
            -0.5f, -0.5f,  0.5f,    0.0f, 0.0f,
            // back
            -0.5f, -0.5f, -0.5f,    1.0f, 0.0f,
             0.5f, -0.5f, -0.5f,    0.0f, 0.0f,
             0.5f,  0.5f, -0.5f,    0.0f, 1.0f,
             0.5f,  0.5f, -0.5f,    0.0f, 1.0f,
            -0.5f,  0.5f, -0.5f,    1.0f, 1.0f,
            -0.5f, -0.5f, -0.5f,    1.0f, 0.0f,
            // left
            -0.5f,  0.5f,  0.5f,    1.0f, 0.0f,
            -0.5f,  0.5f, -0.5f,    1.0f, 1.0f,
            -0.5f, -0.5f, -0.5f,    0.0f, 1.0f,
            -0.5f, -0.5f, -0.5f,    0.0f, 1.0f,
            -0.5f, -0.5f,  0.5f,    0.0f, 0.0f,
            -0.5f,  0.5f,  0.5f,    1.0f, 0.0f,
            // right
             0.5f,  0.5f,  0.5f,    0.0f, 0.0f,
             0.5f,  0.5f, -0.5f,    0.0f, 1.0f,
             0.5f, -0.5f, -0.5f,    1.0f, 1.0f,
             0.5f, -0.5f, -0.5f,    1.0f, 1.0f,
             0.5f, -0.5f,  0.5f,    1.0f, 0.0f,
             0.5f,  0.5f,  0.5f,    0.0f, 0.0f,
            // bottom
            -0.5f, -0.5f, -0.5f,    0.0f, 1.0f,
             0.5f, -0.5f, -0.5f,    1.0f, 1.0f,
             0.5f, -0.5f,  0.5f,    1.0f, 0.0f,
             0.5f, -0.5f,  0.5f,    1.0f, 0.0f,
            -0.5f, -0.5f,  0.5f,    0.0f, 0.0f,
            -0.5f, -0.5f, -0.5f,    0.0f, 1.0f,
            // top
            -0.5f,  0.5f, -0.5f,    0.0f, 1.0f,
             0.5f,  0.5f, -0.5f,    1.0f, 1.0f,
             0.5f,  0.5f,  0.5f,    1.0f, 0.0f,
             0.5f,  0.5f,  0.5f,    1.0f, 0.0f,
            -0.5f,  0.5f,  0.5f,    0.0f, 0.0f,
            -0.5f,  0.5f, -0.5f,    0.0f, 1.0f,
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
            0, // pos
            3,
            static_cast<int>(5 * sizeof(float)),
            0
        );
        vao_.set_attribute_float(
            1, // uv
            2,
            static_cast<int>(5 * sizeof(float)),
            3 * sizeof(float)
        );
        render::VertexArray::unbind();
        render::VertexBuffer::unbind();

        const auto shader_dir = std::filesystem::path(__FILE__).parent_path();
        auto shader_result = render::Shader::from_files(
            shader_dir / "cube.vert",
            shader_dir / "cube.frag"
        );
        if (!shader_result) {
            std::println(std::cerr, "Failed to create shader for HelloCube: {}", shader_result.error());
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

        ctx_.set_depth_test(true);
    }

    void on_update(core::DeltaTime dt, const platform::InputState&) {
        time_ += dt.seconds;
        blend_ = 0.5f + 0.5f * std::sin(time_ * 0.5f);
        bounce_offset_ = 0.25f * std::sin(time_ * bounce_speed_);
        rotation_angle_ += rotation_speed_ * dt.seconds;
    }

    void on_render() {
        ctx_.begin_frame(render::FrameClear{0.05f, 0.05f, 0.08f, 1.0f});

        if (!vao_.is_valid() || shader_.id() == 0 || !texture_a_.is_valid() || !texture_b_.is_valid()) {
            return;
        }

        const core::Mat4 model = core::mul(
            translate(core::Mat4{1.0f}, core::Vec3{0.0f, bounce_offset_, 0.0f}),
            rotate(core::Mat4{1.0f}, rotation_angle_, rotation_axis_)
        );
        const core::Mat4 view  = lookAt(core::Vec3{0.0f, 0.0f, 3.0f}, core::Vec3{0.0f}, core::Vec3{0.0f, 1.0f, 0.0f});
        const core::Mat4 proj  = perspective(radians(45.0f), aspect_ratio_, 0.1f, 100.0f);
        const core::Mat4 mvp   = core::mul(core::mul(proj, view), model);

        shader_.use();
        if (auto loc = gpu::gl::get_uniform_location(shader_.id(), "uMVP"); loc != -1) {
            gpu::gl::set_uniform_mat4(loc, value_ptr(mvp));
        }
        if (auto loc = gpu::gl::get_uniform_location(shader_.id(), "uBlend"); loc != -1) {
            gpu::gl::set_uniform(loc, blend_);
        }
        if (auto loc = gpu::gl::get_uniform_location(shader_.id(), "uTex0"); loc != -1) {
            gpu::gl::set_uniform(loc, 0);
        }
        if (auto loc = gpu::gl::get_uniform_location(shader_.id(), "uTex1"); loc != -1) {
            gpu::gl::set_uniform(loc, 1);
        }

        texture_a_.bind(0);
        texture_b_.bind(1);
        vao_.bind();
        gpu::gl::draw_arrays(gpu::gl::Primitive::triangles, 0, 36);
        render::VertexArray::unbind();
    }

    void on_gui() {}

    void on_resize(int width, int height) {
        ctx_.set_viewport(width, height);
        aspect_ratio_ = height > 0 ? static_cast<float>(width) / static_cast<float>(height) : 1.0f;
    }

private:
    render::Context     ctx_{};
    float             time_{0.0f};
    float             blend_{0.5f};
    float             bounce_offset_{0.0f};
    float             rotation_angle_{0.0f};
    float             rotation_speed_{0.7f};
    float             bounce_speed_{1.2f};
    core::Vec3        rotation_axis_{0.0f, 1.0f, 0.0f};
    float             aspect_ratio_{16.0f / 9.0f};
    render::VertexArray vao_{};
    render::VertexBuffer vbo_{};
    render::Texture2D    texture_a_{};
    render::Texture2D    texture_b_{};
    render::Shader       shader_{};
};

}
