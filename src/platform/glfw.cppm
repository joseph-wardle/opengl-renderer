module;

#include <GLFW/glfw3.h>

export module platform.glfw;

import std;
import gpu.gl;

export namespace platform {

class GlfwContext {
public:
    GlfwContext() {
        glfwSetErrorCallback([](int code, const char* desc) {
            std::println(std::cerr, "GLFW error {}: {}", code, desc);
        });

        if (glfwInit() == GLFW_TRUE) {
            valid_ = true;
            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
            glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        }
    }

    ~GlfwContext() {
        if (valid_) {
            glfwTerminate();
        }
    }

    [[nodiscard]] bool is_valid() const noexcept { return valid_; }

private:
    bool valid_{false};
};

struct WindowConfig {
    int         width{1280};
    int         height{720};
    std::string title{"LearnOpenGL"};
};

using ResizeCallback = void(*)(int, int, void*);
using KeyCallback    = void(*)(int, int, int, int, void*);

class Window {
public:
    Window() = default;

    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    Window(Window&& other) noexcept { move_from(other); }
    Window& operator=(Window&& other) noexcept {
        if (this != &other) {
            destroy();
            move_from(other);
        }
        return *this;
    }

    ~Window() { destroy(); }

    struct Error {
        std::string message;
    };

    [[nodiscard]] static std::expected<Window, Error> create(const WindowConfig& cfg) {
        GLFWwindow* handle = glfwCreateWindow(
            cfg.width,
            cfg.height,
            cfg.title.c_str(),
            nullptr,
            nullptr
        );

        if (!handle) {
            return std::unexpected(Error{"glfwCreateWindow failed"});
        }
        glfwMakeContextCurrent(handle);

        Window window;
        window.handle_ = handle;
        window.refresh_user_pointer();
        window.install_callbacks();
        return window;
    }

    void set_vsync(bool enabled) {
        glfwSwapInterval(enabled ? 1 : 0);
    }

    [[nodiscard]] bool should_close() const {
        return glfwWindowShouldClose(handle_) == GLFW_TRUE;
    }

    void request_close() { glfwSetWindowShouldClose(handle_, GLFW_TRUE); }

    void swap_buffers() { glfwSwapBuffers(handle_); }
    void poll_events() { glfwPollEvents(); }

    void set_resize_callback(ResizeCallback cb, void* userdata = nullptr) noexcept {
        resize_callback_ = cb;
        resize_userdata_ = userdata;
    }

    void set_key_callback(KeyCallback cb, void* userdata = nullptr) noexcept {
        key_callback_ = cb;
        key_userdata_ = userdata;
    }

    using LoadProc = GLFWglproc (*)(const char*);

    [[nodiscard]] LoadProc get_load_proc() const noexcept {
        return glfwGetProcAddress;
    }

private:
    GLFWwindow*    handle_{nullptr};
    ResizeCallback resize_callback_{};
    KeyCallback    key_callback_{};
    void*          resize_userdata_{nullptr};
    void*          key_userdata_{nullptr};

    void refresh_user_pointer() noexcept {
        if (handle_) {
            glfwSetWindowUserPointer(handle_, this);
        }
    }

    void install_callbacks() noexcept {
        glfwSetFramebufferSizeCallback(
            handle_,
            [](GLFWwindow* win, int width, int height) {
                auto* self = static_cast<Window*>(glfwGetWindowUserPointer(win));
                if (self && self->resize_callback_) {
                    self->resize_callback_(width, height, self->resize_userdata_);
                }
                gpu::gl::viewport(0, 0, width, height);
            });

        glfwSetKeyCallback(
            handle_,
            [](GLFWwindow* win, int key, int scancode, int action, int mods) {
                auto* self =
                    static_cast<Window*>(glfwGetWindowUserPointer(win));
                if (self && self->key_callback_) {
                    self->key_callback_(key, scancode, action, mods, self->key_userdata_);
                }
            }
        );
    }

    void destroy() noexcept {
        if (handle_) {
            glfwDestroyWindow(handle_);
            handle_ = nullptr;
        }
        resize_callback_ = nullptr;
        key_callback_ = nullptr;
        resize_userdata_ = nullptr;
        key_userdata_ = nullptr;
    }

    void move_from(Window& other) noexcept {
        handle_ = std::exchange(other.handle_, nullptr);
        resize_callback_ = other.resize_callback_;
        key_callback_ = other.key_callback_;
        resize_userdata_ = other.resize_userdata_;
        key_userdata_ = other.key_userdata_;
        if (handle_) {
            refresh_user_pointer();
            install_callbacks();
        }
        other.resize_callback_ = nullptr;
        other.key_callback_ = nullptr;
        other.resize_userdata_ = nullptr;
        other.key_userdata_ = nullptr;
    }
};

enum class Key : int {
    unknown = -1,
    escape  = GLFW_KEY_ESCAPE,
    w       = GLFW_KEY_W,
    a       = GLFW_KEY_A,
    s       = GLFW_KEY_S,
    d       = GLFW_KEY_D,
};

struct InputState {
    static constexpr int max_keys = GLFW_KEY_LAST + 1;

    std::array<bool, max_keys> current{};
    std::array<bool, max_keys> previous{};

    void begin_frame() noexcept {
        previous = current;
    }

    void handle_key_event(int key, int action) noexcept {
        if (key < 0 || key >= max_keys) return;

        if (action == GLFW_PRESS) {
            current[static_cast<std::size_t>(key)] = true;
        } else if (action == GLFW_RELEASE) {
            current[static_cast<std::size_t>(key)] = false;
        }
    }

    [[nodiscard]] bool is_down(Key key) const noexcept {
        const auto k = static_cast<int>(key);
        if (k < 0 || k >= max_keys) return false;
        return current[static_cast<std::size_t>(k)];
    }

    [[nodiscard]] bool is_pressed(Key key) const noexcept {
        const auto k = static_cast<int>(key);
        if (k < 0 || k >= max_keys) return false;
        const auto idx = static_cast<std::size_t>(k);
        return current[idx] && !previous[idx];
    }

    [[nodiscard]] bool is_released(Key key) const noexcept {
        const auto k = static_cast<int>(key);
        if (k < 0 || k >= max_keys) return false;
        const auto idx = static_cast<std::size_t>(k);
        return !current[idx] && previous[idx];
    }
};

} // namespace platform
