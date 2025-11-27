module;

export module scenes.hello_camera;

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
import render.camera;
import resources.image;
import ui.imgui;

export namespace scenes {

struct CubeInstance {
    core::Vec3 position;
    core::Vec3 axis;
    float      rot_speed;
    float      bounce_speed;
    float      bounce_phase;
};

struct HelloCamera {
    render::Context ctx{};
    void on_init() {
        ctx.set_depth_test(true);

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
            std::println(std::cerr, "Failed to create shader for HelloCamera: {}", shader_result.error());
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

        setup_instances();

        camera_ = render::Camera(core::Vec3{0.0f, 0.0f, 6.0f}, aspect_ratio_);
    }

    void on_update(core::DeltaTime dt, const platform::InputState& input) {
        if (animate_cubes_) {
            anim_time_ += dt.seconds;
            blend_ = 0.5f + 0.5f * std::sin(anim_time_ * 0.4f);
        }

        auto& io = imgui::GetIO();
        const bool allow_camera_input = !io.WantCaptureKeyboard && !io.WantCaptureMouse;
        const bool look_active = input.is_mouse_down(platform::MouseButton::right);
        if (allow_camera_input && look_active) {
            camera_.update(dt, input);
        }
    }

    void on_render() {
        ctx.begin_frame(render::FrameClear{0.05f, 0.05f, 0.08f, 1.0f});

        if (!vao_.is_valid() || shader_.id() == 0 || !texture_a_.is_valid() || !texture_b_.is_valid()) {
            return;
        }

        ctx.set_wireframe(wireframe_);

        const auto view = camera_.view();
        const auto proj = camera_.projection();

        shader_.use();
        if (auto loc = gpu::gl::get_uniform_location(shader_.id(), "uTex0"); loc != -1) {
            gpu::gl::set_uniform(loc, 0);
        }
        if (auto loc = gpu::gl::get_uniform_location(shader_.id(), "uTex1"); loc != -1) {
            gpu::gl::set_uniform(loc, 1);
        }

        texture_a_.bind(0);
        texture_b_.bind(1);
        vao_.bind();

        for (const auto& cube : cubes_) {
            const float bounce = 0.25f * std::sin(anim_time_ * cube.bounce_speed + cube.bounce_phase);
            const core::Mat4 model =
                core::mul(
                    translate(core::Mat4{1.0f}, cube.position + core::Vec3{0.0f, bounce, 0.0f}),
                    rotate(core::Mat4{1.0f}, anim_time_ * cube.rot_speed, cube.axis)
                );
            const core::Mat4 mvp = core::mul(core::mul(proj, view), model);

            if (auto loc = gpu::gl::get_uniform_location(shader_.id(), "uMVP"); loc != -1) {
                gpu::gl::set_uniform_mat4(loc, value_ptr(mvp));
            }
            if (auto loc = gpu::gl::get_uniform_location(shader_.id(), "uBlend"); loc != -1) {
                gpu::gl::set_uniform(loc, blend_);
            }

            gpu::gl::draw_arrays(gpu::gl::Primitive::triangles, 0, 36);
        }

        render::VertexArray::unbind();
        ctx.set_wireframe(false);
    }

    void on_gui() {
        if (!show_debug_window_) {
            return;
        }

        if (imgui::Begin("Camera Debug", &show_debug_window_)) {
            auto& io = imgui::GetIO();
            imgui::Text("FPS: %.1f", io.Framerate);
            imgui::Separator();

            imgui::Checkbox("Animate cubes", &animate_cubes_);
            imgui::Checkbox("Wireframe", &wireframe_);

            if (animate_cubes_) {
                imgui::BeginDisabled();
            }
            imgui::SliderFloat("Blend", &blend_, 0.0f, 1.0f);
            if (animate_cubes_) {
                imgui::EndDisabled();
            }

            float fov = camera_.fov();
            if (imgui::SliderFloat("FOV", &fov, 30.0f, 100.0f)) {
                camera_.set_fov(fov);
            }

            imgui::Text(
                "Input captured: %s",
                (io.WantCaptureKeyboard || io.WantCaptureMouse) ? "ImGui" : "Scene"
            );
        }
        imgui::End();
    }

    void on_resize(int width, int height) {
        ctx.set_viewport(width, height);
        aspect_ratio_ = height > 0 ? static_cast<float>(width) / static_cast<float>(height) : 1.0f;
        camera_.set_aspect(aspect_ratio_);
    }

private:
    void setup_instances() {
        std::mt19937 rng{std::random_device{}()};
        std::uniform_real_distribution<float> axis_dist{-1.0f, 1.0f};
        std::uniform_real_distribution<float> speed_dist{0.4f, 1.2f};
        std::uniform_real_distribution<float> phase_dist{0.0f, two_pi<float>()};

        const int grid = 5;
        const float spacing = 2.0f;
        cubes_.clear();
        cubes_.reserve(static_cast<std::size_t>(grid * grid));
        for (int z = 0; z < grid; ++z) {
            for (int x = 0; x < grid; ++x) {
                core::Vec3 pos{
                    (x - grid / 2) * spacing,
                    0.0f,
                    (z - grid / 2) * spacing
                };
                core::Vec3 axis{axis_dist(rng), axis_dist(rng), axis_dist(rng)};
                if (length(axis) < 0.001f) axis = core::Vec3{0.0f, 1.0f, 0.0f};
                axis = normalize(axis);
                cubes_.push_back(CubeInstance{
                    pos,
                    axis,
                    speed_dist(rng),
                    speed_dist(rng),
                    phase_dist(rng)
                });
            }
        }
    }

    float                anim_time_{0.0f};
    float                blend_{0.5f};
    float                aspect_ratio_{16.0f / 9.0f};
    bool                 show_debug_window_{true};
    bool                 animate_cubes_{true};
    bool                 wireframe_{false};
    render::VertexArray  vao_{};
    render::VertexBuffer vbo_{};
    render::Texture2D    texture_a_{};
    render::Texture2D    texture_b_{};
    render::Shader       shader_{};
    render::Camera       camera_{core::Vec3{0.0f, 0.0f, 6.0f}, aspect_ratio_};
    std::vector<CubeInstance> cubes_;
};

}
