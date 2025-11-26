export module core.app;

import std;
import platform.glfw;
import gpu.gl;
import ui.imgui_layer;

export namespace core {

struct AppConfig {
    int         width{1280};
    int         height{720};
    std::string title{"LearnOpenGL"};
    bool        vsync{true};
};

using Clock = std::chrono::steady_clock;

struct DeltaTime {
    float seconds{0.0f};
};

template <typename Scene>
concept SceneConcept =
    requires(
        Scene& scene,
        DeltaTime dt,
        const platform::InputState& input,
        int w,
        int h
    ) {
        { scene.on_init() }            -> std::same_as<void>;
        { scene.on_update(dt, input) } -> std::same_as<void>;
        { scene.on_render() }          -> std::same_as<void>;
        { scene.on_resize(w, h) }      -> std::same_as<void>;
        { scene.on_gui() }             -> std::same_as<void>;
    };

template <SceneConcept Scene>
class Application {
public:
    Application(AppConfig config, Scene scene)
        : config_(std::move(config)), scene_(std::move(scene)) {}

    int run();

private:
    AppConfig config_;
    Scene scene_;
};

// Implementation in the same module for now.

template <SceneConcept Scene>
int Application<Scene>::run() {
    platform::GlfwContext glfw{};
    if (!glfw.is_valid()) {
        return 1;
    }

    platform::WindowConfig wc;
    wc.width  = config_.width;
    wc.height = config_.height;
    wc.title  = config_.title;

    auto window_expected = platform::Window::create(wc);
    if (!window_expected) {
        const auto& err = window_expected.error();
        // std::println(std::cerr, "Window creation failed:");
        std::println(std::cerr, "    Error code: {}", err.code);
        std::println(std::cerr, "    Error message: {}", err.message);
        return 1;
    }
    platform::Window window = std::move(*window_expected);

    window.set_resize_callback(
        [](int w, int h, void* user) {
            if (auto* scene = static_cast<Scene*>(user)) {
                scene->on_resize(w, h);
            }
        },
        &scene_
    );

    platform::InputState input{};
    window.set_key_callback(
        [](int key, int scancode, int action, int mods, void* user) {
            (void)scancode; (void)mods; // unused for now
            if (auto* state = static_cast<platform::InputState*>(user)) {
                state->handle_key_event(key, action);
            }
        },
        &input
    );

    window.set_cursor_pos_callback(
        [](double x, double y, void* user) {
            if (auto* state = static_cast<platform::InputState*>(user)) {
                state->handle_cursor_pos(x, y);
            }
        },
        &input
    );

    // Initialize GLAD
    if (!gpu::gl::init(window.get_load_proc())) {
        std::println(std::cerr, "Failed to initialize OpenGL loader (GLAD)");
        gpu::gl::shutdown();
        return 1;
    }

    window.set_vsync(config_.vsync);
    gpu::gl::viewport(0, 0, config_.width, config_.height);

    scene_.on_init();
    scene_.on_resize(config_.width, config_.height);

    ui::ImGuiLayer imgui{window};
    bool show_debug_ui = true;

    auto last_time = Clock::now();

    while (!window.should_close()) {
        input.begin_frame();
        window.poll_events();

        auto now  = Clock::now();
        auto dt   = std::chrono::duration<float>(now - last_time).count();
        last_time = now;

        if (input.is_down(platform::Key::escape)) {
            window.request_close();
        }
        if (input.is_pressed(platform::Key::f1)) {
            show_debug_ui = !show_debug_ui;
        }

        imgui.begin_frame();
        scene_.on_update(DeltaTime{dt}, input);
        scene_.on_render();
        if (show_debug_ui) {
            scene_.on_gui();
        }
        imgui.end_frame();

        window.swap_buffers();
    }

    gpu::gl::shutdown();

    return 0;
}

} // namespace core
