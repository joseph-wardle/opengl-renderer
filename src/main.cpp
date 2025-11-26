import core.app;
import scenes.phong_maps;

auto main() -> int {
    core::AppConfig cfg{
        .vsync = false,
    };
    core::Application app{cfg, scenes::PhongMaps{}};
    return app.run();
}
