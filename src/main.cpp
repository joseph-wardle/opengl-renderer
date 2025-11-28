import core.app;
import scenes.hello_obj;

auto main() -> int {
    core::AppConfig cfg{
        .vsync = false,
    };
    core::Application app{cfg, scenes::HelloObj{}};
    return app.run();
}
