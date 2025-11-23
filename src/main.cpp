import core.app;
import demo.hello_window;

auto main() -> int {
    core::Application app{core::AppConfig{}, demos::HelloWindow{}};
    return app.run();
}
