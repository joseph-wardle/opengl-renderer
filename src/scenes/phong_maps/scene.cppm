export module scenes.phong_maps;

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
import render.texture;
import resources.image;
import ui.imgui;

export namespace scenes {

struct PhongMaps {
    void on_init() {
        gpu::gl::enable_depth_test(true);

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
        point_light_.position = core::Vec3{2.0f, 3.0f, 2.0f};
        point_light_.intensity = 2.0f;
        point_light_.linear = 0.045f;
        point_light_.quadratic = 0.0075f;
        dir_light_.intensity = 0.3f;
        dir_light_.direction = core::Vec3{-0.3f, -1.0f, -0.2f};
        spot_light_.intensity = 0.0f; // off by default
    }

    void on_update(core::DeltaTime dt, const platform::InputState& input) {
        time_ += dt.seconds;
        if (input.is_mouse_down(platform::MouseButton::right)) {
            camera_.update(dt, input);
        }
    }

    void on_render() {
        gpu::gl::clear_color(0.02f, 0.02f, 0.04f, 1.0f);
        gpu::gl::clear(gpu::gl::COLOR_BUFFER_BIT | gpu::gl::DEPTH_BUFFER_BIT);

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
        if (auto loc = gpu::gl::get_uniform_location(shader_.id(), "uDiffuseMap"); loc != -1) {
            gpu::gl::set_uniform(loc, 0);
        }
        if (auto loc = gpu::gl::get_uniform_location(shader_.id(), "uSpecularMap"); loc != -1) {
            gpu::gl::set_uniform(loc, 1);
        }
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
        gpu::gl::viewport(0, 0, width, height);
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
        // Cube with normals and UVs (same layout as PhongCube)
        const std::array<render::Vertex, 24> cube_vertices{{
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
            0, 1, 2, 0, 2, 3,
            4, 5, 6, 4, 6, 7,
            8, 9,10, 8,10,11,
            12,13,14, 12,14,15,
            16,17,18, 16,18,19,
            20,21,22, 20,22,23
        };

        auto mesh_res = render::Mesh::from_data(cube_vertices, cube_indices);
        if (!mesh_res) {
            std::println(std::cerr, "Failed to build cube mesh: {}", mesh_res.error());
            return false;
        }
        cube_mesh_ = std::move(*mesh_res);

        // Floor
        const std::array<render::Vertex, 4> floor_vertices{{
            {{-8.0f, -1.0f,  8.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
            {{ 8.0f, -1.0f,  8.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
            {{ 8.0f, -1.0f, -8.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}},
            {{-8.0f, -1.0f, -8.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}},
        }};
        const std::array<std::uint32_t, 6> floor_indices{
            0, 1, 2,
            0, 2, 3
        };

        auto floor_res = render::Mesh::from_data(floor_vertices, floor_indices);
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
        const auto program = shader.id();
        if (auto loc = gpu::gl::get_uniform_location(program, "uView"); loc != -1) {
            gpu::gl::set_uniform_mat4(loc, value_ptr(view));
        }
        if (auto loc = gpu::gl::get_uniform_location(program, "uProjection"); loc != -1) {
            gpu::gl::set_uniform_mat4(loc, value_ptr(proj));
        }
        if (auto loc = gpu::gl::get_uniform_location(program, "uViewPos"); loc != -1) {
            gpu::gl::set_uniform_vec3(loc, value_ptr(camera_.position()));
        }
        dir_light_.apply(shader, "uDirLight");
        point_light_.apply(shader, "uPointLight");
        spot_light_.apply(shader, "uSpotLight");
    }

    void draw_mesh(const render::Shader& shader, const render::Mesh& mesh, const core::Mat4& model, const render::PhongMaterial& material, const core::Vec3& color = core::Vec3{1.0f}) {
        const auto program = shader.id();
        if (auto loc = gpu::gl::get_uniform_location(program, "uModel"); loc != -1) {
            gpu::gl::set_uniform_mat4(loc, value_ptr(model));
        }
        const core::Mat3 normal_matrix = transpose(inverse(core::Mat3(model)));
        if (auto loc = gpu::gl::get_uniform_location(program, "uNormalMatrix"); loc != -1) {
            gpu::gl::set_uniform_mat3(loc, value_ptr(normal_matrix));
        }
        if (auto loc = gpu::gl::get_uniform_location(program, "uColor"); loc != -1) {
            gpu::gl::set_uniform_vec3(loc, value_ptr(color));
        }
        material.apply(shader, "uMaterial");
        mesh.draw();
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
    render::DirectionalLight dir_light_{};
    render::PointLight       point_light_{};
    render::SpotLight        spot_light_{};
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
