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

using ResizeCallback       = void(*)(int, int, void*);
using KeyCallback          = void(*)(int, int, int, int, void*);
using CursorPosCallback    = void(*)(double, double, void*);
using MouseButtonCallback  = void(*)(int, int, int, void*);
using ScrollCallback       = void(*)(double, double, void*);

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
        int         code{0};
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
            const char* desc = nullptr;
            const int code = glfwGetError(&desc);
            std::string msg = desc ? desc : "glfwCreateWindow failed";
            return std::unexpected(Error{code, msg});
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

    void set_cursor_pos_callback(CursorPosCallback cb, void* userdata = nullptr) noexcept {
        cursor_pos_callback_ = cb;
        cursor_pos_userdata_ = userdata;
    }

    void set_mouse_button_callback(MouseButtonCallback cb, void* userdata = nullptr) noexcept {
        mouse_button_callback_ = cb;
        mouse_button_userdata_ = userdata;
    }

    void set_scroll_callback(ScrollCallback cb, void* userdata = nullptr) noexcept {
        scroll_callback_ = cb;
        scroll_userdata_ = userdata;
    }

    void set_cursor_disabled(bool disabled) noexcept {
        if (handle_) {
            glfwSetInputMode(handle_, GLFW_CURSOR, disabled ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
        }
    }

    using LoadProc = GLFWglproc (*)(const char*);

    [[nodiscard]] LoadProc get_load_proc() const noexcept {
        return glfwGetProcAddress;
    }

    [[nodiscard]] GLFWwindow* native_handle() const noexcept {
        return handle_;
    }

private:
    GLFWwindow*    handle_{nullptr};
    ResizeCallback resize_callback_{};
    KeyCallback    key_callback_{};
    CursorPosCallback   cursor_pos_callback_{};
    MouseButtonCallback mouse_button_callback_{};
    ScrollCallback      scroll_callback_{};
    void*          resize_userdata_{nullptr};
    void*          key_userdata_{nullptr};
    void*          cursor_pos_userdata_{nullptr};
    void*          mouse_button_userdata_{nullptr};
    void*          scroll_userdata_{nullptr};

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

        glfwSetCursorPosCallback(
            handle_,
            [](GLFWwindow* win, double x, double y) {
                auto* self = static_cast<Window*>(glfwGetWindowUserPointer(win));
                if (self && self->cursor_pos_callback_) {
                    self->cursor_pos_callback_(x, y, self->cursor_pos_userdata_);
                }
            }
        );

        glfwSetScrollCallback(
            handle_,
            [](GLFWwindow* win, double xoffset, double yoffset) {
                auto* self = static_cast<Window*>(glfwGetWindowUserPointer(win));
                if (self && self->scroll_callback_) {
                    self->scroll_callback_(xoffset, yoffset, self->scroll_userdata_);
                }
            }
        );

        glfwSetMouseButtonCallback(
            handle_,
            [](GLFWwindow* win, int button, int action, int mods) {
                (void)mods;
                auto* self = static_cast<Window*>(glfwGetWindowUserPointer(win));
                if (self && self->mouse_button_callback_) {
                    self->mouse_button_callback_(button, action, mods, self->mouse_button_userdata_);
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
        cursor_pos_callback_ = nullptr;
        mouse_button_callback_ = nullptr;
        scroll_callback_ = nullptr;
        resize_userdata_ = nullptr;
        key_userdata_ = nullptr;
        cursor_pos_userdata_ = nullptr;
        mouse_button_userdata_ = nullptr;
        scroll_userdata_ = nullptr;
    }

    void move_from(Window& other) noexcept {
        handle_ = std::exchange(other.handle_, nullptr);
        resize_callback_ = other.resize_callback_;
        key_callback_ = other.key_callback_;
        cursor_pos_callback_ = other.cursor_pos_callback_;
        mouse_button_callback_ = other.mouse_button_callback_;
        scroll_callback_ = other.scroll_callback_;
        resize_userdata_ = other.resize_userdata_;
        key_userdata_ = other.key_userdata_;
        cursor_pos_userdata_ = other.cursor_pos_userdata_;
        mouse_button_userdata_ = other.mouse_button_userdata_;
        scroll_userdata_ = other.scroll_userdata_;
        if (handle_) {
            refresh_user_pointer();
            install_callbacks();
        }
        other.resize_callback_ = nullptr;
        other.key_callback_ = nullptr;
        other.cursor_pos_callback_ = nullptr;
        other.mouse_button_callback_ = nullptr;
        other.scroll_callback_ = nullptr;
        other.resize_userdata_ = nullptr;
        other.key_userdata_ = nullptr;
        other.cursor_pos_userdata_ = nullptr;
        other.mouse_button_userdata_ = nullptr;
        other.scroll_userdata_ = nullptr;
    }
};

enum class Key : int {
    unknown = -1,
    escape  = GLFW_KEY_ESCAPE,
    w       = GLFW_KEY_W,
    a       = GLFW_KEY_A,
    s       = GLFW_KEY_S,
    d       = GLFW_KEY_D,
    q       = GLFW_KEY_Q,
    e       = GLFW_KEY_E,
    up      = GLFW_KEY_UP,
    down    = GLFW_KEY_DOWN,
    left    = GLFW_KEY_LEFT,
    right   = GLFW_KEY_RIGHT,
    left_shift = GLFW_KEY_LEFT_SHIFT,
    f1      = GLFW_KEY_F1,
};

enum class MouseButton : int {
    left   = GLFW_MOUSE_BUTTON_LEFT,
    right  = GLFW_MOUSE_BUTTON_RIGHT,
    middle = GLFW_MOUSE_BUTTON_MIDDLE,
};

struct InputState {
    static constexpr int max_keys = GLFW_KEY_LAST + 1;
    static constexpr int max_mouse_buttons = GLFW_MOUSE_BUTTON_LAST + 1;

    std::array<bool, max_keys> current{};
    std::array<bool, max_keys> previous{};
    std::array<bool, max_mouse_buttons> mouse_current{};
    std::array<bool, max_mouse_buttons> mouse_previous{};

    struct MouseDelta {
        double x{0.0};
        double y{0.0};
    };

    struct MouseState {
        double x{0.0};
        double y{0.0};
        MouseDelta delta{};
        bool first{true};
    } mouse{};

    struct ScrollDelta {
        double x{0.0};
        double y{0.0};
    } scroll{};

    void begin_frame() noexcept {
        previous = current;
        mouse_previous = mouse_current;
        mouse.delta = {};
        scroll = {};
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

    void handle_cursor_pos(double x, double y) noexcept {
        if (mouse.first) {
            mouse.x = x;
            mouse.y = y;
            mouse.first = false;
            return;
        }
        mouse.delta.x += x - mouse.x;
        mouse.delta.y += y - mouse.y;
        mouse.x = x;
        mouse.y = y;
    }

    void handle_scroll(double xoffset, double yoffset) noexcept {
        scroll.x += xoffset;
        scroll.y += yoffset;
    }

    [[nodiscard]] MouseDelta mouse_delta() const noexcept { return mouse.delta; }
    [[nodiscard]] ScrollDelta scroll_delta() const noexcept { return scroll; }

    void handle_mouse_button(int button, int action) noexcept {
        if (button < 0 || button >= max_mouse_buttons) return;
        const auto idx = static_cast<std::size_t>(button);
        if (action == GLFW_PRESS) {
            mouse_current[idx] = true;
        } else if (action == GLFW_RELEASE) {
            mouse_current[idx] = false;
            mouse.first = true; // avoid large deltas on next press
        }
    }

    [[nodiscard]] bool is_mouse_down(MouseButton button) const noexcept {
        const auto b = static_cast<int>(button);
        if (b < 0 || b >= max_mouse_buttons) return false;
        return mouse_current[static_cast<std::size_t>(b)];
    }

    [[nodiscard]] bool is_mouse_pressed(MouseButton button) const noexcept {
        const auto b = static_cast<int>(button);
        if (b < 0 || b >= max_mouse_buttons) return false;
        const auto idx = static_cast<std::size_t>(b);
        return mouse_current[idx] && !mouse_previous[idx];
    }
};

} // namespace platform
