import core.app;
import scenes.hello_textures;

auto main() -> int {
    core::AppConfig cfg{};
    core::Application app{cfg, scenes::HelloTextured{false}};
    return app.run();
}
