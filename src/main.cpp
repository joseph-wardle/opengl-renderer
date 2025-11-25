import core.app;
import scenes.hello_cube;

auto main() -> int {
    core::AppConfig cfg{};
    core::Application app{cfg, scenes::HelloCube{}};
    return app.run();
}
