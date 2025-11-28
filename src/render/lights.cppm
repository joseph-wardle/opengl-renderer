export module render.lights;

import std;
import core.glm;
import render.shader;

export namespace render {

struct DirectionalLight {
    core::Vec3 direction{ -0.2f, -1.0f, -0.3f };
    core::Vec3 color{1.0f};
    float      intensity{0.0f}; // disabled by default

    void apply(const Shader& shader, std::string_view prefix) const {
        std::string base{prefix};
        shader.set_vec3(base + ".direction", direction);
        shader.set_vec3(base + ".color", color);
        shader.set_float(base + ".intensity", intensity);
    }

    void apply_at(const Shader& shader, std::string_view array_name, int index) const {
        std::string base{array_name};
        base.push_back('[');
        base.append(std::to_string(index));
        base.push_back(']');
        apply(shader, base);
    }
};

struct PointLight {
    core::Vec3 position{0.0f, 1.5f, 1.0f};
    core::Vec3 color{1.0f};
    float      intensity{1.0f};
    float      constant{1.0f};
    float      linear{0.09f};
    float      quadratic{0.032f};

    void apply(const Shader& shader, std::string_view prefix) const {
        std::string base{prefix};
        shader.set_vec3(base + ".position", position);
        shader.set_vec3(base + ".color", color);
        shader.set_float(base + ".intensity", intensity);
        shader.set_float(base + ".constant", constant);
        shader.set_float(base + ".linear", linear);
        shader.set_float(base + ".quadratic", quadratic);
    }

    void apply_at(const Shader& shader, std::string_view array_name, int index) const {
        std::string base{array_name};
        base.push_back('[');
        base.append(std::to_string(index));
        base.push_back(']');
        apply(shader, base);
    }
};

struct SpotLight {
    core::Vec3 position{0.0f, 2.5f, 0.0f};
    core::Vec3 direction{0.0f, -1.0f, 0.0f};
    core::Vec3 color{1.0f};
    float      intensity{0.0f}; // disabled by default
    float      inner_cos{std::cos(12.5f * static_cast<float>(std::numbers::pi) / 180.0f)};
    float      outer_cos{std::cos(17.5f * static_cast<float>(std::numbers::pi) / 180.0f)};
    float      constant{1.0f};
    float      linear{0.09f};
    float      quadratic{0.032f};

    void apply(const Shader& shader, std::string_view prefix) const {
        std::string base{prefix};
        shader.set_vec3(base + ".position", position);
        shader.set_vec3(base + ".direction", direction);
        shader.set_vec3(base + ".color", color);
        shader.set_float(base + ".intensity", intensity);
        shader.set_float(base + ".inner_cos", inner_cos);
        shader.set_float(base + ".outer_cos", outer_cos);
        shader.set_float(base + ".constant", constant);
        shader.set_float(base + ".linear", linear);
        shader.set_float(base + ".quadratic", quadratic);
    }

    void apply_at(const Shader& shader, std::string_view array_name, int index) const {
        std::string base{array_name};
        base.push_back('[');
        base.append(std::to_string(index));
        base.push_back(']');
        apply(shader, base);
    }
};

} // namespace render
