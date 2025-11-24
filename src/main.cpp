import core.app;
import scenes.hello_square;

auto main() -> int {
    core::AppConfig cfg{};
    core::Application app{cfg, scenes::HelloSquare{false}};
    return app.run();
}
