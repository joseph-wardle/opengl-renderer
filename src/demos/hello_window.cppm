export module demo.hello_window;

import gpu.gl;
import core.app;
import platform.glfw;

export namespace demos {

struct HelloWindow {
    void on_update(core::DeltaTime, const platform::InputState&) {
        // handle input later
    }

    void on_render() {
        gpu::gl::clear_color(0.2f, 0.3f, 0.3f, 1.0f);
        gpu::gl::clear(gpu::gl::COLOR_BUFFER_BIT);
    }

    void on_resize(int, int) {}
};

}
