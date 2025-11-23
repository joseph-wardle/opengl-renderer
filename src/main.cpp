import core.app;

int main() {
    core::AppConfig config {
        .width = 1280,
        .height = 720,
        .title = "LearnOpenGL",
        .vsync = true
    };
    core::Application app(config);
    app.run();
    return 0;
}