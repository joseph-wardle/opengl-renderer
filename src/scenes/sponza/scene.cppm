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
        ctx_.set_backface_culling(true);
        std::println("Sponza init: starting");

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
        std::println("Sponza init: shader loaded");

        const auto obj_path = shader_dir / "sponza.obj";
        auto model_res = render::load_obj_model(obj_path);
        if (!model_res) {
            std::println(std::cerr, "Failed to load Sponza OBJ {}: {}", obj_path.string(), model_res.error().message);
            return;
        }
        model_ = std::move(*model_res);
        std::println("Sponza init: model loaded parts={}", model_.parts.size());
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
        if (!logged_render_start_) {
            std::println("Sponza render: begin");
            logged_render_start_ = true;
        }

        const auto view = camera_.view();
        const auto proj = camera_.projection();
        if (!logged_first_frame_) {
            std::println("Sponza render: view/projection computed");
        }

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
        if (!logged_first_frame_) {
            std::println("Sponza render: shader uniforms set");
        }

        auto draw_part = [&](const render::Model::Part& part) {
            shader_.set_vec3("uBaseColor", part.base_color);
            shader_.set_int("uHasDiffuseMap", part.has_diffuse() ? 1 : 0);
            shader_.set_int("uHasSpecularMap", part.has_specular() ? 1 : 0);
            if (part.has_diffuse()) part.diffuse_map->bind(0);
            if (part.has_specular()) part.specular_map->bind(1);
            part.mesh.draw();
        };
        if (!logged_first_frame_) {
            std::println("Sponza render: draw_part lambda ready");
        }

        if (!logged_first_frame_) std::println("Sponza render: clearing queues");
        std::vector<const render::Model::Part*> opaque_queue;
        std::vector<TransparentDraw>            transparent_queue;
        opaque_queue.reserve(model_.parts.size());
        transparent_queue.reserve(model_.parts.size());
        if (!logged_first_frame_) {
            std::println("Sponza render: building queues for {} parts", model_.parts.size());
        }

        std::size_t debug_count = 0;
        for (const auto& part : model_.parts) {
            if (part.is_transparent()) {
                const core::Vec3 world_center = core::Vec3(model_mat * core::Vec4(part.bounds_center, 1.0f));
                float distance = length(camera_.position() - world_center);
                if (!std::isfinite(distance)) {
                    distance = 0.0f;
                }
                transparent_queue.push_back(TransparentDraw{&part, distance});
                if (!logged_first_frame_ && debug_count < 3) {
                    std::println(
                        "  queued transparent part idx={} alpha_mode={} distance={}",
                        debug_count,
                        static_cast<int>(part.alpha_mode),
                        distance
                    );
                }
            } else {
                opaque_queue.push_back(&part);
                if (!logged_first_frame_ && debug_count < 3) {
                    std::println(
                        "  queued opaque part idx={} alpha_mode={}",
                        debug_count,
                        static_cast<int>(part.alpha_mode)
                    );
                }
            }
            ++debug_count;
        }
        if (!logged_first_frame_) {
            std::println(
                "Sponza render: queues built opaque={} transparent={}",
                opaque_queue.size(),
                transparent_queue.size()
            );
        }

        for (const auto* part : opaque_queue) {
            draw_part(*part);
        }
        if (!logged_first_frame_) {
            std::println("Sponza render: opaque pass done");
        }

        if (!transparent_queue.empty()) {
            std::ranges::sort(
                transparent_queue,
                [](const TransparentDraw& a, const TransparentDraw& b) {
                    return a.distance > b.distance;
                }
            );

            ctx_.set_alpha_blending(true);
            ctx_.set_depth_write(false);
            for (const auto& draw : transparent_queue) {
                draw_part(*draw.part);
            }
            ctx_.set_depth_write(true);
            ctx_.set_alpha_blending(false);
            if (!logged_first_frame_) {
                std::println("Sponza render: transparent pass done");
            }
        }

        if (!logged_first_frame_) {
            std::println(
                "Sponza render first frame: opaque={} transparent={}",
                opaque_queue.size(),
                transparent_queue.size()
            );
            logged_first_frame_ = true;
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
    struct TransparentDraw {
        const render::Model::Part* part{nullptr};
        float                      distance{0.0f};
    };

    render::Context ctx_{};
    render::Model   model_{};
    render::Shader  shader_{};
    float           aspect_{16.0f / 9.0f};
    render::Camera  camera_{core::Vec3{0.0f, 1.0f, 6.0f}, aspect_};
    bool            loaded_{false};
    bool            logged_first_frame_{false};
    bool            logged_render_start_{false};
};

} // namespace scenes
