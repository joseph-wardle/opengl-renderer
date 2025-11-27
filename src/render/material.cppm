export module render.material;

import std;
import core.glm;
import render.shader;
import render.uniforms;

export namespace render {

struct PhongMaterial {
    core::Vec3 ambient{0.1f};
    core::Vec3 diffuse{0.8f};
    core::Vec3 specular{0.5f};
    float      shininess{32.0f};

    void apply(const Shader& shader, std::string_view prefix) const {
        const auto ambient_name  = uniforms::with_field(prefix, ".ambient");
        const auto diffuse_name  = uniforms::with_field(prefix, ".diffuse");
        const auto specular_name = uniforms::with_field(prefix, ".specular");
        const auto shininess_name = uniforms::with_field(prefix, ".shininess");

        uniforms::set_vec3(shader, ambient_name, ambient);
        uniforms::set_vec3(shader, diffuse_name, diffuse);
        uniforms::set_vec3(shader, specular_name, specular);
        uniforms::set_float(shader, shininess_name, shininess);
    }
};

} // namespace render
