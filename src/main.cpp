import core.app;
import scenes.hello_camera;

auto main() -> int {
    core::AppConfig cfg{};
    core::Application app{cfg, scenes::HelloCamera{}};
    return app.run();
}
