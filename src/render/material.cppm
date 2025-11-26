export module render.material;

import std;
import core.glm;
import gpu.gl;
import render.shader;

export namespace render {

struct PhongMaterial {
    core::Vec3 ambient{0.1f};
    core::Vec3 diffuse{0.8f};
    core::Vec3 specular{0.5f};
    float      shininess{32.0f};

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

        set_vec3(".ambient", ambient);
        set_vec3(".diffuse", diffuse);
        set_vec3(".specular", specular);
        set_float(".shininess", shininess);
    }
};

} // namespace render
