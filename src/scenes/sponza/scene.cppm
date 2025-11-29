export module scenes.sponza;

import std;
import render.context;
import core.app;
import core.glm;
import platform.glfw;
import render.shader;
import render.model;
import render.camera;
import ui.imgui;

export namespace scenes {

struct Sponza {
    void on_init() {
        ctx_.set_depth_test(true);

        const auto shader_dir = std::filesystem::path(__FILE__).parent_path();
        auto shader_res = render::Shader::from_files(
            shader_dir / "sponza.vert",
            shader_dir / "sponza.frag"
        );
        if (!shader_res) {
            std::println(std::cerr, "Failed to create Sponza shader: {}", shader_res.error());
            return;
        }
        shader_ = std::move(*shader_res);

        const auto obj_path = shader_dir / "sponza.obj";
        auto model_res = render::load_obj_model(obj_path);
        if (!model_res) {
            std::println(std::cerr, "Failed to load Sponza OBJ {}: {}", obj_path.string(), model_res.error().message);
            return;
        }
        model_ = std::move(*model_res);
        loaded_ = true;
    }

    void on_update(core::DeltaTime dt, const platform::InputState& input, bool allow_input) {
        if (allow_input && input.is_mouse_down(platform::MouseButton::right)) {
            camera_.update(dt, input);
        }
    }

    void on_render() {
        ctx_.begin_frame(render::FrameClear{0.04f, 0.05f, 0.06f, 1.0f});

        if (!loaded_ || shader_.id() == 0) {
            return;
        }

        const auto view = camera_.view();
        const auto proj = camera_.projection();

        shader_.use();
        shader_.set_mat4("uView", view);
        shader_.set_mat4("uProjection", proj);
        shader_.set_vec3("uLightDir", normalize(core::Vec3{-0.2f, -1.0f, -0.3f}));
        shader_.set_vec3("uLightColor", core::Vec3{1.0f});
        shader_.set_vec3("uViewPos", camera_.position());

        const core::Mat4 model_mat = scale(core::Mat4{1.0f}, core::Vec3{0.01f});
        const core::Mat3 normal_mat = transpose(inverse(core::Mat3(model_mat)));
        shader_.set_mat4("uModel", model_mat);
        shader_.set_mat3("uNormalMatrix", normal_mat);
        shader_.set_int("uDiffuseMap", 0);
        shader_.set_int("uSpecularMap", 1);

        for (const auto& part : model_.parts) {
            shader_.set_vec3("uBaseColor", part.base_color);
            shader_.set_int("uHasDiffuseMap", part.has_diffuse() ? 1 : 0);
            shader_.set_int("uHasSpecularMap", part.has_specular() ? 1 : 0);
            if (part.has_diffuse()) part.diffuse_map->bind(0);
            if (part.has_specular()) part.specular_map->bind(1);
            part.mesh.draw();
        }
    }

    void on_resize(int width, int height) {
        ctx_.set_viewport(width, height);
        aspect_ = height > 0 ? static_cast<float>(width) / static_cast<float>(height) : 1.0f;
        camera_.set_aspect(aspect_);
    }

    void on_gui() {
        if (imgui::Begin("Sponza")) {
            imgui::Text("Loaded: %s", loaded_ ? "yes" : "no");
            imgui::Text("Parts: %zu", model_.parts.size());
            imgui::Text("FPS: %.1f", imgui::GetIO().Framerate);
        }
        imgui::End();
    }

private:
    render::Context ctx_{};
    render::Model   model_{};
    render::Shader  shader_{};
    float           aspect_{16.0f / 9.0f};
    render::Camera  camera_{core::Vec3{0.0f, 1.0f, 6.0f}, aspect_};
    bool            loaded_{false};
};

} // namespace scenes
