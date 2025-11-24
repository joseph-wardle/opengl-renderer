import core.app;
import scene.hello_window;

auto main() -> int {
    core::Application app{core::AppConfig{}, scenes::HelloWindow{}};
    return app.run();
}
