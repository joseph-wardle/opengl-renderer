export module render.lights;

import std;
import core.glm;
import render.shader;
import render.uniforms;

export namespace render {

struct DirectionalLight {
    core::Vec3 direction{ -0.2f, -1.0f, -0.3f };
    core::Vec3 color{1.0f};
    float      intensity{0.0f}; // disabled by default

    void apply(const Shader& shader, std::string_view prefix) const {
        uniforms::set_vec3(shader, uniforms::with_field(prefix, ".direction"), direction);
        uniforms::set_vec3(shader, uniforms::with_field(prefix, ".color"), color);
        uniforms::set_float(shader, uniforms::with_field(prefix, ".intensity"), intensity);
    }

    void apply_at(const Shader& shader, std::string_view array_name, int index) const {
        apply(shader, uniforms::index_into(array_name, index));
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
        uniforms::set_vec3(shader, uniforms::with_field(prefix, ".position"), position);
        uniforms::set_vec3(shader, uniforms::with_field(prefix, ".color"), color);
        uniforms::set_float(shader, uniforms::with_field(prefix, ".intensity"), intensity);
        uniforms::set_float(shader, uniforms::with_field(prefix, ".constant"), constant);
        uniforms::set_float(shader, uniforms::with_field(prefix, ".linear"), linear);
        uniforms::set_float(shader, uniforms::with_field(prefix, ".quadratic"), quadratic);
    }

    void apply_at(const Shader& shader, std::string_view array_name, int index) const {
        apply(shader, uniforms::index_into(array_name, index));
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
        uniforms::set_vec3(shader, uniforms::with_field(prefix, ".position"), position);
        uniforms::set_vec3(shader, uniforms::with_field(prefix, ".direction"), direction);
        uniforms::set_vec3(shader, uniforms::with_field(prefix, ".color"), color);
        uniforms::set_float(shader, uniforms::with_field(prefix, ".intensity"), intensity);
        uniforms::set_float(shader, uniforms::with_field(prefix, ".inner_cos"), inner_cos);
        uniforms::set_float(shader, uniforms::with_field(prefix, ".outer_cos"), outer_cos);
        uniforms::set_float(shader, uniforms::with_field(prefix, ".constant"), constant);
        uniforms::set_float(shader, uniforms::with_field(prefix, ".linear"), linear);
        uniforms::set_float(shader, uniforms::with_field(prefix, ".quadratic"), quadratic);
    }

    void apply_at(const Shader& shader, std::string_view array_name, int index) const {
        apply(shader, uniforms::index_into(array_name, index));
    }
};

} // namespace render
