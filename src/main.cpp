import core.app;
import scenes.sponza;

auto main() -> int {
    core::AppConfig cfg{
        .vsync = false,
    };
    core::Application app{cfg, scenes::Sponza{}};
    return app.run();
}
