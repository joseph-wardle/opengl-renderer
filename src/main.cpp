import core.app;
import scenes.phong_cube;

auto main() -> int {
    core::AppConfig cfg{
        .vsync = false,
    };
    core::Application app{cfg, scenes::PhongCube{}};
    return app.run();
}
