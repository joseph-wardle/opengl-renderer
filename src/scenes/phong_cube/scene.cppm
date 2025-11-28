export module scenes.phong_cube;

import std;
import core.app;
import core.glm;
import render.context;
import render.primitives;
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
    render::Context ctx{};
    void on_init() {
        ctx.set_depth_test(true);

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

    void on_update(core::DeltaTime dt, const platform::InputState& input, bool allow_input) {
        time_ += dt.seconds;
        if (allow_input && input.is_mouse_down(platform::MouseButton::right)) {
            camera_.update(dt, input);
        }
    }

    void on_render() {
        ctx.begin_frame(render::FrameClear{0.02f, 0.02f, 0.04f, 1.0f});

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
        ctx.set_viewport(width, height);
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
        auto cube_mesh = render::make_unit_cube();
        if (!cube_mesh) {
            std::println(std::cerr, "Failed to build cube mesh: {}", cube_mesh.error());
            return false;
        }
        cube_mesh_ = std::move(*cube_mesh);

        auto floor_mesh = render::make_floor_quad(5.0f, -0.5f);
        if (!floor_mesh) {
            std::println(std::cerr, "Failed to build floor mesh: {}", floor_mesh.error());
            return false;
        }
        floor_mesh_ = std::move(*floor_mesh);

        return true;
    }

    void upload_common_uniforms(const core::Mat4& view, const core::Mat4& proj) {
        shader_.set_mat4("uView", view);
        shader_.set_mat4("uProjection", proj);
        shader_.set_vec3("uViewPos", camera_.position());
        light_.apply(shader_, "uLight");
    }

    void draw_mesh(const render::Mesh& mesh, const core::Mat4& model, const core::Vec3& base_color, const render::PhongMaterial& material) {
        const core::Mat3 normal_matrix = transpose(inverse(core::Mat3(model)));
        shader_.set_mat4("uModel", model);
        shader_.set_mat3("uNormalMatrix", normal_matrix);
        shader_.set_vec3("uBaseColor", base_color);

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
