export module render.lights;

import std;
import core.glm;
import gpu.gl;
import render.shader;

export namespace render {

namespace detail {
inline void set_vec3(const Shader& shader, std::string_view name, const core::Vec3& v) {
    if (auto loc = gpu::gl::get_uniform_location(shader.id(), std::string(name).c_str()); loc != -1) {
        gpu::gl::set_uniform_vec3(loc, value_ptr(v));
    }
}
inline void set_float(const Shader& shader, std::string_view name, float v) {
    if (auto loc = gpu::gl::get_uniform_location(shader.id(), std::string(name).c_str()); loc != -1) {
        gpu::gl::set_uniform(loc, v);
    }
}
} // namespace detail

struct DirectionalLight {
    core::Vec3 direction{ -0.2f, -1.0f, -0.3f };
    core::Vec3 color{1.0f};
    float      intensity{0.0f}; // disabled by default

    void apply(const Shader& shader, std::string_view prefix) const {
        detail::set_vec3(shader, std::string(prefix).append(".direction"), direction);
        detail::set_vec3(shader, std::string(prefix).append(".color"), color);
        detail::set_float(shader, std::string(prefix).append(".intensity"), intensity);
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
        detail::set_vec3(shader, std::string(prefix).append(".position"), position);
        detail::set_vec3(shader, std::string(prefix).append(".color"), color);
        detail::set_float(shader, std::string(prefix).append(".intensity"), intensity);
        detail::set_float(shader, std::string(prefix).append(".constant"), constant);
        detail::set_float(shader, std::string(prefix).append(".linear"), linear);
        detail::set_float(shader, std::string(prefix).append(".quadratic"), quadratic);
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
        detail::set_vec3(shader, std::string(prefix).append(".position"), position);
        detail::set_vec3(shader, std::string(prefix).append(".direction"), direction);
        detail::set_vec3(shader, std::string(prefix).append(".color"), color);
        detail::set_float(shader, std::string(prefix).append(".intensity"), intensity);
        detail::set_float(shader, std::string(prefix).append(".inner_cos"), inner_cos);
        detail::set_float(shader, std::string(prefix).append(".outer_cos"), outer_cos);
        detail::set_float(shader, std::string(prefix).append(".constant"), constant);
        detail::set_float(shader, std::string(prefix).append(".linear"), linear);
        detail::set_float(shader, std::string(prefix).append(".quadratic"), quadratic);
    }
};

} // namespace render
