export module core.app;

import std;
import platform.glfw;
import gpu.gl;

export namespace core {

struct AppConfig {
    int width{1280};
    int height{720};
    std::string_view title{"LearnOpenGL"};
    bool vsync{true};
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

    // Initialize GLAD
    if (!gpu::gl::init(window.get_load_proc())) {
        return 1;
    }

    window.set_vsync(config_.vsync);
    gpu::gl::viewport(0, 0, config_.width, config_.height);
    
    scene_.on_init();
    scene_.on_resize(config_.width, config_.height);

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

        scene_.on_update(DeltaTime{dt}, input);

        gpu::gl::clear(gpu::gl::COLOR_BUFFER_BIT | gpu::gl::DEPTH_BUFFER_BIT);
        scene_.on_render();

        window.swap_buffers();
    }

    return 0;
}

} // namespace core
