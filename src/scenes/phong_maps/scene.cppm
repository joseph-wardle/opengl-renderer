export module scenes.phong_maps;

import std;
import core.app;
import core.glm;
import render.context;
import render.uniforms;
import render.primitives;
import gpu.gl;
import platform.glfw;
import render.shader;
import render.mesh;
import render.material;
import render.lights;
import render.camera;
import render.texture;
import resources.image;
import ui.imgui;

export namespace scenes {

namespace uniforms = render::uniforms;

struct PhongMaps {
    render::Context ctx{};
    void on_init() {
        ctx.set_depth_test(true);

        if (!create_meshes()) {
            return;
        }

        const auto shader_dir = std::filesystem::path(__FILE__).parent_path();
        auto shader_res = render::Shader::from_files(
            shader_dir / "phong_maps.vert",
            shader_dir / "phong_maps.frag"
        );
        if (!shader_res) {
            std::println(std::cerr, "Failed to create PhongMaps shader: {}", shader_res.error());
            return;
        }
        shader_ = std::move(*shader_res);

        auto floor_shader_res = render::Shader::from_files(
            shader_dir / "floor.vert",
            shader_dir / "floor.frag"
        );
        if (!floor_shader_res) {
            std::println(std::cerr, "Failed to create floor shader: {}", floor_shader_res.error());
            return;
        }
        floor_shader_ = std::move(*floor_shader_res);

        if (!load_textures(shader_dir)) {
            return;
        }

        camera_ = render::Camera(core::Vec3{0.0f, 1.5f, 5.0f}, aspect_);
        if (dir_count_ < dir_capacity) {
            dir_lights_[dir_count_++] = render::DirectionalLight{
                .direction = core::Vec3{-0.3f, -1.0f, -0.2f},
                .color = core::Vec3{1.0f},
                .intensity = 0.3f
            };
        }
        if (point_count_ < point_capacity) {
            point_lights_[point_count_++] = render::PointLight{
                .position = core::Vec3{2.0f, 3.0f, 2.0f},
                .color = core::Vec3{1.0f},
                .intensity = 2.0f,
                .constant = 1.0f,
                .linear = 0.045f,
                .quadratic = 0.0075f
            };
        }
        // Spot off by default; add here if desired.
    }

    void on_update(core::DeltaTime dt, const platform::InputState& input) {
        time_ += dt.seconds;
        if (input.is_mouse_down(platform::MouseButton::right)) {
            camera_.update(dt, input);
        }
    }

    void on_render() {
        ctx.begin_frame(render::FrameClear{0.02f, 0.02f, 0.04f, 1.0f});

        if (!shader_.id() || !floor_shader_.id()
            || !cube_mesh_.is_valid() || !floor_mesh_.is_valid()
            || !diffuse_map_.is_valid() || !specular_map_.is_valid()) {
            return;
        }

        const auto view = camera_.view();
        const auto proj = camera_.projection();

        // Cube with textures
        shader_.use();
        upload_common_uniforms(shader_, view, proj);
        diffuse_map_.bind(0);
        specular_map_.bind(1);
        uniforms::set_int(shader_, "uDiffuseMap", 0);
        uniforms::set_int(shader_, "uSpecularMap", 1);
        core::Mat4 cube_model = core::mul(
            translate(core::Mat4{1.0f}, core::Vec3{0.0f, 0.75f, 0.0f}),
            rotate(core::Mat4{1.0f}, time_ * 0.6f, core::Vec3{0.0f, 1.0f, 0.0f})
        );
        draw_mesh(shader_, cube_mesh_, cube_model, cube_material_);

        // Floor (solid color)
        floor_shader_.use();
        upload_common_uniforms(floor_shader_, view, proj);
        core::Mat4 floor_model = core::Mat4{1.0f};
        draw_mesh(floor_shader_, floor_mesh_, floor_model, floor_material_, floor_color_);
    }

    void on_resize(int width, int height) {
        ctx.set_viewport(width, height);
        aspect_ = height > 0 ? static_cast<float>(width) / static_cast<float>(height) : 1.0f;
        camera_.set_aspect(aspect_);
    }

    void on_gui() {
        if (imgui::Begin("Phong Maps")) {
            imgui::Text("FPS: %.1f", imgui::GetIO().Framerate);
        }
        imgui::End();
    }

private:
    bool create_meshes() {
        auto mesh_res = render::make_unit_cube();
        if (!mesh_res) {
            std::println(std::cerr, "Failed to build cube mesh: {}", mesh_res.error());
            return false;
        }
        cube_mesh_ = std::move(*mesh_res);

        // Floor
        auto floor_res = render::make_floor_quad(8.0f, -1.0f);
        if (!floor_res) {
            std::println(std::cerr, "Failed to build floor mesh: {}", floor_res.error());
            return false;
        }
        floor_mesh_ = std::move(*floor_res);

        return true;
    }

    bool load_textures(const std::filesystem::path& dir) {
        auto diffuse_img = resources::load_image((dir / "diffuse.png").string());
        if (!diffuse_img) {
            std::println(std::cerr, "Failed to load diffuse map: {}", diffuse_img.error());
            return false;
        }
        auto specular_img = resources::load_image((dir / "specular.png").string());
        if (!specular_img) {
            std::println(std::cerr, "Failed to load specular map: {}", specular_img.error());
            return false;
        }

        auto diffuse_tex = render::Texture2D::from_image(*diffuse_img, true);
        if (!diffuse_tex) {
            std::println(std::cerr, "Failed to create diffuse texture: {}", diffuse_tex.error());
            return false;
        }
        diffuse_map_ = std::move(*diffuse_tex);

        auto specular_tex = render::Texture2D::from_image(*specular_img, true);
        if (!specular_tex) {
            std::println(std::cerr, "Failed to create specular texture: {}", specular_tex.error());
            return false;
        }
        specular_map_ = std::move(*specular_tex);
        return true;
    }

    void upload_common_uniforms(const render::Shader& shader, const core::Mat4& view, const core::Mat4& proj) {
        uniforms::set_mat4(shader, "uView", view);
        uniforms::set_mat4(shader, "uProjection", proj);
        uniforms::set_vec3(shader, "uViewPos", camera_.position());
        // Upload light counts
        detail_upload_lights(shader);
    }

    void draw_mesh(const render::Shader& shader, const render::Mesh& mesh, const core::Mat4& model, const render::PhongMaterial& material, const core::Vec3& color = core::Vec3{1.0f}) {
        const core::Mat3 normal_matrix = transpose(inverse(core::Mat3(model)));
        uniforms::set_mat4(shader, "uModel", model);
        uniforms::set_mat3(shader, "uNormalMatrix", normal_matrix);
        uniforms::set_vec3(shader, "uColor", color);
        material.apply(shader, "uMaterial");
        mesh.draw();
    }

    void detail_upload_lights(const render::Shader& shader) {
        uniforms::set_int(shader, "uDirCount", dir_count_);
        uniforms::set_int(shader, "uPointCount", point_count_);
        uniforms::set_int(shader, "uSpotCount", spot_count_);

        for (int i = 0; i < dir_count_; ++i) {
            dir_lights_[static_cast<std::size_t>(i)].apply_at(shader, "uDirLights", i);
        }
        for (int i = 0; i < point_count_; ++i) {
            point_lights_[static_cast<std::size_t>(i)].apply_at(shader, "uPointLights", i);
        }
        for (int i = 0; i < spot_count_; ++i) {
            spot_lights_[static_cast<std::size_t>(i)].apply_at(shader, "uSpotLights", i);
        }
    }

    float time_{0.0f};
    float aspect_{16.0f / 9.0f};

    render::Mesh       cube_mesh_{};
    render::Mesh       floor_mesh_{};
    render::Shader     shader_{};
    render::Shader     floor_shader_{};
    render::Texture2D  diffuse_map_{};
    render::Texture2D  specular_map_{};
    render::Camera     camera_{core::Vec3{0.0f, 1.5f, 5.0f}, aspect_};
    static constexpr int dir_capacity{4};
    static constexpr int point_capacity{4};
    static constexpr int spot_capacity{2};
    std::array<render::DirectionalLight, dir_capacity> dir_lights_{};
    std::array<render::PointLight, point_capacity>     point_lights_{};
    std::array<render::SpotLight, spot_capacity>       spot_lights_{};
    int dir_count_{0};
    int point_count_{0};
    int spot_count_{0};
    render::PhongMaterial cube_material_{
        .ambient = core::Vec3{0.2f},
        .diffuse = core::Vec3{0.8f},
        .specular = core::Vec3{1.0f},
        .shininess = 32.0f,
    };
    render::PhongMaterial floor_material_{
        .ambient = core::Vec3{0.3f},
        .diffuse = core::Vec3{0.7f},
        .specular = core::Vec3{0.15f},
        .shininess = 8.0f,
    };
    core::Vec3 floor_color_{0.35f, 0.45f, 0.5f};
};

} // namespace scenes
