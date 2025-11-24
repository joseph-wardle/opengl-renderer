import core.app;
import scenes.hello_triangle;

auto main() -> int {
    core::AppConfig cfg{};
    core::Application app{cfg, scenes::HelloTriangle{}};
    return app.run();
}
