import core.app;
import scenes.hello_triangle;

auto main() -> int {
    core::Application app{core::AppConfig{}, scenes::HelloTriangle{}};
    return app.run();
}
