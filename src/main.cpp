import core.app;
import scenes.hello_square;

auto main() -> int {
    core::Application app{core::AppConfig{}, scenes::HelloSquare{false}};
    return app.run();
}
