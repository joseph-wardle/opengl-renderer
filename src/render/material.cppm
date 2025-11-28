export module render.material;

import std;
import core.glm;
import render.shader;

export namespace render {

struct PhongMaterial {
    core::Vec3 ambient{0.1f};
    core::Vec3 diffuse{0.8f};
    core::Vec3 specular{0.5f};
    float      shininess{32.0f};

    void apply(const Shader& shader, std::string_view prefix) const {
        std::string base{prefix};
        shader.set_vec3(base + ".ambient", ambient);
        shader.set_vec3(base + ".diffuse", diffuse);
        shader.set_vec3(base + ".specular", specular);
        shader.set_float(base + ".shininess", shininess);
    }
};

} // namespace render
