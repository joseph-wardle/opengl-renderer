export module render.uniforms;

import std;
import core.glm;
import render.shader;
import gpu.gl;

export namespace render::uniforms {

namespace detail {
struct UniformKey {
    gpu::gl::ProgramId program{};
    std::string        name;

    friend bool operator==(const UniformKey& a, const UniformKey& b) noexcept {
        return a.program == b.program && a.name == b.name;
    }
};

struct UniformKeyHash {
    std::size_t operator()(const UniformKey& key) const noexcept {
        const auto h_prog = std::hash<gpu::gl::ProgramId>{}(key.program);
        const auto h_name = std::hash<std::string>{}(key.name);
        return h_prog ^ (h_name + 0x9e3779b97f4a7c15ULL + (h_prog << 6) + (h_prog >> 2));
    }
};

inline gpu::gl::UniformLocation locate(const Shader& shader, std::string_view name) {
    static std::unordered_map<UniformKey, gpu::gl::UniformLocation, UniformKeyHash> cache;
    static std::mutex cache_mutex;

    std::string owned{name};
    const UniformKey key{shader.id(), owned};

    {
        std::scoped_lock lock{cache_mutex};
        if (const auto it = cache.find(key); it != cache.end()) {
            return it->second;
        }
    }

    const auto loc = gpu::gl::get_uniform_location(shader.id(), owned);
    {
        std::scoped_lock lock{cache_mutex};
        cache.emplace(key, loc);
    }
    return loc;
}

inline std::string join(std::string_view prefix, std::string_view field) {
    std::string name;
    name.reserve(prefix.size() + field.size());
    name.append(prefix);
    name.append(field);
    return name;
}

inline std::string indexed(std::string_view array_name, int index) {
    std::string name;
    name.reserve(array_name.size() + 6);
    name.append(array_name);
    name.push_back('[');
    name.append(std::to_string(index));
    name.push_back(']');
    return name;
}
} // namespace detail

inline bool set_int(const Shader& shader, std::string_view name, int value) {
    if (auto loc = detail::locate(shader, name); loc != -1) {
        gpu::gl::set_uniform(loc, value);
        return true;
    }
    return false;
}

inline bool set_float(const Shader& shader, std::string_view name, float value) {
    if (auto loc = detail::locate(shader, name); loc != -1) {
        gpu::gl::set_uniform(loc, value);
        return true;
    }
    return false;
}

inline bool set_vec3(const Shader& shader, std::string_view name, const core::Vec3& value) {
    if (auto loc = detail::locate(shader, name); loc != -1) {
        gpu::gl::set_uniform_vec3(loc, value_ptr(value));
        return true;
    }
    return false;
}

inline bool set_mat3(const Shader& shader, std::string_view name, const core::Mat3& value) {
    if (auto loc = detail::locate(shader, name); loc != -1) {
        gpu::gl::set_uniform_mat3(loc, value_ptr(value));
        return true;
    }
    return false;
}

inline bool set_mat4(const Shader& shader, std::string_view name, const core::Mat4& value) {
    if (auto loc = detail::locate(shader, name); loc != -1) {
        gpu::gl::set_uniform_mat4(loc, value_ptr(value));
        return true;
    }
    return false;
}

inline std::string with_field(std::string_view prefix, std::string_view field) {
    return detail::join(prefix, field);
}

inline std::string index_into(std::string_view array_name, int index) {
    return detail::indexed(array_name, index);
}

} // namespace render::uniforms
