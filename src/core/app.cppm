export module core.app;

import std;

export namespace core {

struct AppConfig {
    int width{1280};
    int height{720};
    std::string_view title{"LearnOpenGL"};
    bool vsync{true};
};

class Application {
public:
    Application(AppConfig config) : config_(std::move(config)) {}

    int run();

private:
    AppConfig config_;
};

int Application::run() {
    std::println("Starting application with title: '{}', size: {}x{}, vsync: {}",
        config_.title, config_.width, config_.height, config_.vsync);
    return 0;
}

} // namespace core
