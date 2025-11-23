---

# main.cpp

```cpp
import core.app;
import demo.hello_window;

auto main() -> int {
  const core::AppConfig config{
    .width  = 1280,
    .height = 720,
    .title  = "LearnOpenGL",
    .vsync  = true,
  };

  demos::HelloWindow hello_window_demo{};

  core::Application app{config, hello_window_demo};
  return app.run();
}
```

---

# core/app.cppm

```cpp
export module core.app;

import std;
import platform.glfw;
import gpu.gl;
import math.glm;

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

  template <typename Demo>
  concept DemoConcept =
    requires(Demo& d, DeltaTime dt, int w, int h) {
      { d.on_update(dt) } -> std::same_as<void>;
      { d.on_render() }   -> std::same_as<void>;
      { d.on_resize(w, h) } -> std::same_as<void>;
    };

  template <DemoConcept Demo>
  class Application {
  public:
    Application(AppConfig config, Demo demo)
      : config_(std::move(config)), demo_(std::move(demo)) {}

    int run();

  private:
    AppConfig config_;
    Demo demo_;
  };

  // Implementation in the same module for now.

  template <DemoConcept Demo>
  int Application<Demo>::run() {
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

    // Hook resize callback to forward into demo
    window.set_resize_callback(
      [this](int w, int h) { demo_.on_resize(w, h); });

    // Initialize GLAD through our gpu.gl wrapper
    if (!gpu::gl::init(window.get_load_proc())) {
      return 1;
    }

    window.set_vsync(config_.vsync);
    gpu::gl::viewport(0, 0, config_.width, config_.height);

    auto last_time = Clock::now();

    while (!window.should_close()) {
      auto now = Clock::now();
      auto dt  = std::chrono::duration<float>(now - last_time).count();
      last_time = now;

      demo_.on_update(DeltaTime{dt});

      gpu::gl::clear(gpu::gl::COLOR_BUFFER_BIT | gpu::gl::DEPTH_BUFFER_BIT);
      demo_.on_render();

      window.swap_buffers();
      window.poll_events();
    }

    return 0;
  }

} // namespace core
```

---

# platform/glfw.cppm

```cpp
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
    static std::optional<Window> create(const WindowConfig& cfg) {
        GLFWwindow* handle =
            glfwCreateWindow(cfg.width, cfg.height, cfg.title.data(), nullptr, nullptr);
        if (!handle) {
            return std::nullopt;
        }
        glfwMakeContextCurrent(handle);

        Window w;
        w.handle_ = handle;
        glfwSetWindowUserPointer(handle, &w);

        glfwSetFramebufferSizeCallback(
            handle,
            [](GLFWwindow* win, int w_width, int w_height) {
                auto* self = static_cast<Window*>(glfwGetWindowUserPointer(win));
                if (self && self->resize_callback_) {
                    self->resize_callback_(w_width, w_height);
                }
                gpu::gl::viewport(0, 0, w_width, w_height);
            });

        return w;
    }

    Window() = default;

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
    }

    using LoadProc = GLFWglproc (*)(const char*);
    
    // Function pointer for GLAD
    [[nodiscard]] LoadProc get_load_proc() const {
        return glfwGetProcAddress;
    }

private:
    GLFWwindow*     handle_{nullptr};
    ResizeCallback resize_callback_{};

    void swap(Window& other) noexcept {
        std::swap(handle_, other.handle_);
        std::swap(resize_callback_, other.resize_callback_);
    }
};

} // namespace platform
```

---

# gpu/gl.cppm

```cpp
module;

export module gpu.gl;

#include <glad/gl.h>

export namespace gpu::gl {

  // Re-exported constants (capture macro values)
  constexpr unsigned COLOR_BUFFER_BIT = GL_COLOR_BUFFER_BIT;
  constexpr unsigned DEPTH_BUFFER_BIT = GL_DEPTH_BUFFER_BIT;
  // Add more as needed.

  using LoadFunc = GLADloadfunc;

  [[nodiscard]] inline bool init(LoadFunc load) noexcept {
    if (!load) {
      return false;
    }
    return gladLoadGL(load) != 0;
  }

  inline void viewport(int x, int y, int w, int h) {
    ::glViewport(x, y, w, h);
  }

  inline void clear(unsigned mask) {
    ::glClear(mask);
  }
  
  inline void clear_color(float r, float g, float b, float a) {
    ::glClearColor(r, g, b, a);
  }

} // namespace gpu::gl
```

---

# math/glm.cppm

```cpp
// src/math/math.glm.cppm
module;

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE 0 // keep OpenGL default
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

export module math.glm;

export namespace math {
  using Vec2 = glm::vec2;
  using Vec3 = glm::vec3;
  using Vec4 = glm::vec4;

  using Mat3 = glm::mat3;
  using Mat4 = glm::mat4;

  using glm::radians;
  using glm::value_ptr;
}
```

---

# demos/hello_window.cppm

```cpp
export module demo.hello_window;

import gpu.gl;
import core.app;

export namespace demos {

  struct HelloWindow {
    void on_update(core::DeltaTime) {
      // handle input later
    }

    void on_render() {
      gpu::gl::clear_color(0.2f, 0.3f, 0.3f, 1.0f);
      gpu::gl::clear(gpu::gl::COLOR_BUFFER_BIT);
    }

    void on_resize(int, int) {}
  };

}
```

