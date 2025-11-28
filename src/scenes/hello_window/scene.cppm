export module scenes.hello_window;

import render.context;
import core.app;
import platform.glfw;

export namespace scenes {

struct HelloWindow {
    render::Context ctx{};

    void on_init() {
        // initialization code later
    }
    
    void on_update(core::DeltaTime, const platform::InputState&, bool) {
        // handle input later
    }

    void on_render() {
        ctx.begin_frame(render::FrameClear{0.2f, 0.3f, 0.3f, 1.0f});
    }

    void on_gui() {}

    void on_resize(int, int) {}
};

}
