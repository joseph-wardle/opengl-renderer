export module render.lights;

import std;
import core.glm;
import gpu.gl;
import render.shader;

export namespace render {

struct PointLight {
    core::Vec3 position{0.0f, 1.5f, 1.0f};
    core::Vec3 color{1.0f};
    float      intensity{1.0f};
    float      constant{1.0f};
    float      linear{0.09f};
    float      quadratic{0.032f};

    void apply(const Shader& shader, std::string_view prefix) const {
        const auto program = shader.id();
        auto set_vec3 = [&](std::string_view name, const core::Vec3& v) {
            if (auto loc = gpu::gl::get_uniform_location(program, std::string(prefix).append(name).c_str()); loc != -1) {
                gpu::gl::set_uniform_vec3(loc, value_ptr(v));
            }
        };
        auto set_float = [&](std::string_view name, float v) {
            if (auto loc = gpu::gl::get_uniform_location(program, std::string(prefix).append(name).c_str()); loc != -1) {
                gpu::gl::set_uniform(loc, v);
            }
        };

        set_vec3(".position", position);
        set_vec3(".color", color);
        set_float(".intensity", intensity);
        set_float(".constant", constant);
        set_float(".linear", linear);
        set_float(".quadratic", quadratic);
    }
};

} // namespace render
