export module scenes.phong_cube;

import std;
import core.app;
import core.glm;
import gpu.gl;
import platform.glfw;
import render.shader;
import render.mesh;
import render.material;
import render.lights;
import render.camera;
import ui.imgui;

export namespace scenes {

struct PhongCube {
    void on_init() {
        gpu::gl::enable_depth_test(true);

        if (!create_meshes()) {
            return;
        }

        const auto shader_dir = std::filesystem::path(__FILE__).parent_path();
        auto shader_res = render::Shader::from_files(
            shader_dir / "phong.vert",
            shader_dir / "phong.frag"
        );
        if (!shader_res) {
            std::println(std::cerr, "Failed to create Phong shader: {}", shader_res.error());
            return;
        }
        shader_ = std::move(*shader_res);

        camera_ = render::Camera(core::Vec3{0.0f, 1.5f, 5.0f}, aspect_);
    }

    void on_update(core::DeltaTime dt, const platform::InputState& input) {
        time_ += dt.seconds;
        camera_.update(dt, input);
    }

    void on_render() {
        gpu::gl::clear_color(0.02f, 0.02f, 0.04f, 1.0f);
        gpu::gl::clear(gpu::gl::COLOR_BUFFER_BIT | gpu::gl::DEPTH_BUFFER_BIT);

        if (!shader_.id() || !cube_mesh_.is_valid() || !floor_mesh_.is_valid()) {
            return;
        }

        shader_.use();

        const auto view = camera_.view();
        const auto proj = camera_.projection();

        upload_common_uniforms(view, proj);

        // Cube (slight rotation to show specular highlights)
        core::Mat4 cube_model = core::mul(
            translate(core::Mat4{1.0f}, core::Vec3{0.0f, 0.75f, 0.0f}),
            rotate(core::Mat4{1.0f}, time_ * 0.6f, core::Vec3{0.0f, 1.0f, 0.0f})
        );
        draw_mesh(cube_mesh_, cube_model, cube_color_, cube_material_);

        // Floor
        core::Mat4 floor_model = core::Mat4{1.0f};
        draw_mesh(floor_mesh_, floor_model, floor_color_, floor_material_);
    }

    void on_resize(int width, int height) {
        gpu::gl::viewport(0, 0, width, height);
        aspect_ = height > 0 ? static_cast<float>(width) / static_cast<float>(height) : 1.0f;
        camera_.set_aspect(aspect_);
    }

    void on_gui() {
        if (imgui::Begin("Phong Cube Debug")) {
            imgui::Text("FPS: %.1f", imgui::GetIO().Framerate);
        }
        imgui::End();
    }

private:
    bool create_meshes() {
        // Cube with normals
        const std::array<render::Vertex, 24> cube_vertices{{
            // position                // normal             // uv
            {{-0.5f, -0.5f,  0.5f}, { 0.0f,  0.0f,  1.0f}, {0.0f, 0.0f}},
            {{ 0.5f, -0.5f,  0.5f}, { 0.0f,  0.0f,  1.0f}, {1.0f, 0.0f}},
            {{ 0.5f,  0.5f,  0.5f}, { 0.0f,  0.0f,  1.0f}, {1.0f, 1.0f}},
            {{-0.5f,  0.5f,  0.5f}, { 0.0f,  0.0f,  1.0f}, {0.0f, 1.0f}},

            {{-0.5f, -0.5f, -0.5f}, { 0.0f,  0.0f, -1.0f}, {0.0f, 0.0f}},
            {{ 0.5f, -0.5f, -0.5f}, { 0.0f,  0.0f, -1.0f}, {1.0f, 0.0f}},
            {{ 0.5f,  0.5f, -0.5f}, { 0.0f,  0.0f, -1.0f}, {1.0f, 1.0f}},
            {{-0.5f,  0.5f, -0.5f}, { 0.0f,  0.0f, -1.0f}, {0.0f, 1.0f}},

            {{-0.5f,  0.5f,  0.5f}, {-1.0f,  0.0f,  0.0f}, {1.0f, 0.0f}},
            {{-0.5f,  0.5f, -0.5f}, {-1.0f,  0.0f,  0.0f}, {1.0f, 1.0f}},
            {{-0.5f, -0.5f, -0.5f}, {-1.0f,  0.0f,  0.0f}, {0.0f, 1.0f}},
            {{-0.5f, -0.5f,  0.5f}, {-1.0f,  0.0f,  0.0f}, {0.0f, 0.0f}},

            {{ 0.5f,  0.5f,  0.5f}, { 1.0f,  0.0f,  0.0f}, {1.0f, 0.0f}},
            {{ 0.5f,  0.5f, -0.5f}, { 1.0f,  0.0f,  0.0f}, {1.0f, 1.0f}},
            {{ 0.5f, -0.5f, -0.5f}, { 1.0f,  0.0f,  0.0f}, {0.0f, 1.0f}},
            {{ 0.5f, -0.5f,  0.5f}, { 1.0f,  0.0f,  0.0f}, {0.0f, 0.0f}},

            {{-0.5f, -0.5f, -0.5f}, { 0.0f, -1.0f,  0.0f}, {0.0f, 1.0f}},
            {{ 0.5f, -0.5f, -0.5f}, { 0.0f, -1.0f,  0.0f}, {1.0f, 1.0f}},
            {{ 0.5f, -0.5f,  0.5f}, { 0.0f, -1.0f,  0.0f}, {1.0f, 0.0f}},
            {{-0.5f, -0.5f,  0.5f}, { 0.0f, -1.0f,  0.0f}, {0.0f, 0.0f}},

            {{-0.5f,  0.5f, -0.5f}, { 0.0f,  1.0f,  0.0f}, {0.0f, 1.0f}},
            {{ 0.5f,  0.5f, -0.5f}, { 0.0f,  1.0f,  0.0f}, {1.0f, 1.0f}},
            {{ 0.5f,  0.5f,  0.5f}, { 0.0f,  1.0f,  0.0f}, {1.0f, 0.0f}},
            {{-0.5f,  0.5f,  0.5f}, { 0.0f,  1.0f,  0.0f}, {0.0f, 0.0f}},
        }};

        const std::array<std::uint32_t, 36> cube_indices{
            0, 1, 2, 0, 2, 3,       // front
            4, 5, 6, 4, 6, 7,       // back
            8, 9,10, 8,10,11,       // left
            12,13,14, 12,14,15,     // right
            16,17,18, 16,18,19,     // bottom
            20,21,22, 20,22,23      // top
        };

        auto cube_mesh = render::Mesh::from_data(cube_vertices, cube_indices);
        if (!cube_mesh) {
            std::println(std::cerr, "Failed to build cube mesh: {}", cube_mesh.error());
            return false;
        }
        cube_mesh_ = std::move(*cube_mesh);

        // Simple floor quad
        const std::array<render::Vertex, 4> floor_vertices{{
            {{-5.0f, -0.5f,  5.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
            {{ 5.0f, -0.5f,  5.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
            {{ 5.0f, -0.5f, -5.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}},
            {{-5.0f, -0.5f, -5.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}},
        }};

        const std::array<std::uint32_t, 6> floor_indices{
            0, 1, 2,
            0, 2, 3
        };

        auto floor_mesh = render::Mesh::from_data(floor_vertices, floor_indices);
        if (!floor_mesh) {
            std::println(std::cerr, "Failed to build floor mesh: {}", floor_mesh.error());
            return false;
        }
        floor_mesh_ = std::move(*floor_mesh);

        return true;
    }

    void upload_common_uniforms(const core::Mat4& view, const core::Mat4& proj) {
        const auto program = shader_.id();

        if (auto loc = gpu::gl::get_uniform_location(program, "uView"); loc != -1) {
            gpu::gl::set_uniform_mat4(loc, value_ptr(view));
        }
        if (auto loc = gpu::gl::get_uniform_location(program, "uProjection"); loc != -1) {
            gpu::gl::set_uniform_mat4(loc, value_ptr(proj));
        }
        if (auto loc = gpu::gl::get_uniform_location(program, "uViewPos"); loc != -1) {
            gpu::gl::set_uniform_vec3(loc, value_ptr(camera_.position()));
        }

        light_.apply(shader_, "uLight");
    }

    void draw_mesh(const render::Mesh& mesh, const core::Mat4& model, const core::Vec3& base_color, const render::PhongMaterial& material) {
        const auto program = shader_.id();
        if (auto loc = gpu::gl::get_uniform_location(program, "uModel"); loc != -1) {
            gpu::gl::set_uniform_mat4(loc, value_ptr(model));
        }
        const core::Mat3 normal_matrix = transpose(inverse(core::Mat3(model)));
        if (auto loc = gpu::gl::get_uniform_location(program, "uNormalMatrix"); loc != -1) {
            gpu::gl::set_uniform_mat3(loc, value_ptr(normal_matrix));
        }
        if (auto loc = gpu::gl::get_uniform_location(program, "uBaseColor"); loc != -1) {
            gpu::gl::set_uniform_vec3(loc, value_ptr(base_color));
        }

        material.apply(shader_, "uMaterial");
        mesh.draw();
    }

    float time_{0.0f};
    float aspect_{16.0f / 9.0f};

    render::Mesh         cube_mesh_{};
    render::Mesh         floor_mesh_{};
    render::Shader       shader_{};
    render::Camera       camera_{core::Vec3{0.0f, 1.5f, 5.0f}, aspect_};
    render::PointLight   light_{};
    render::PhongMaterial cube_material_{
        .ambient = core::Vec3{0.15f},
        .diffuse = core::Vec3{0.8f},
        .specular = core::Vec3{0.9f},
        .shininess = 64.0f,
    };
    render::PhongMaterial floor_material_{
        .ambient = core::Vec3{0.2f},
        .diffuse = core::Vec3{0.6f},
        .specular = core::Vec3{0.05f},
        .shininess = 8.0f,
    };
    core::Vec3 cube_color_{0.9f, 0.4f, 0.3f};
    core::Vec3 floor_color_{0.2f, 0.8f, 0.7f};
};

} // namespace scenes
