module;

#include <GLFW/glfw3.h>

export module platform.glfw;

import std;
import gpu.gl;

export namespace platform {

class GlfwContext {
public:
    GlfwContext() {
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
    int width{1280};
    int height{720};
    std::string_view title{"LearnOpenGL"};
};

using ResizeCallback = std::function<void(int, int)>;

class Window {
public:
    Window() = default;

    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    Window(Window&& other) noexcept { swap(other); }
    Window& operator=(Window&& other) noexcept {
        if (this != &other) swap(other);
        return *this;
    }

    ~Window() {
        if (handle_) {
            glfwDestroyWindow(handle_);
        }
    }

    static std::optional<Window> create(const WindowConfig& cfg) {
        GLFWwindow* handle = glfwCreateWindow(
            cfg.width, 
            cfg.height, 
            cfg.title.data(), 
            nullptr, 
            nullptr
        );
        
        if (!handle) {
            return std::nullopt;
        }
        glfwMakeContextCurrent(handle);

        Window window;
        window.handle_ = handle;
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

    void set_resize_callback(ResizeCallback cb) {
        resize_callback_ = std::move(cb);
        glfwSetWindowUserPointer(handle_, this);
        glfwSetFramebufferSizeCallback(
            handle_,
            [](GLFWwindow* win, int width, int height) {
                auto* self =
                    static_cast<Window*>(glfwGetWindowUserPointer(win));
                if (self && self->resize_callback_) {
                    self->resize_callback_(width, height);
                }
                gpu::gl::viewport(0, 0, width, height);
            });
    }

    using LoadProc = GLFWglproc (*)(const char*);

    [[nodiscard]] LoadProc get_load_proc() const noexcept {
        return glfwGetProcAddress;
    }

private:
    GLFWwindow*    handle_{nullptr};
    ResizeCallback resize_callback_{};

    void swap(Window& other) noexcept {
        std::swap(handle_, other.handle_);
        std::swap(resize_callback_, other.resize_callback_);
    }
};

} // namespace platform
