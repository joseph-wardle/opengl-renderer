export module render.shader;

import std;
import gpu.gl;
import core.glm;

export namespace render {

namespace detail {

inline std::expected<std::string, std::string>
load_text_file(const std::filesystem::path& path) {
    std::ifstream file{path, std::ios::in | std::ios::binary};
    if (!file) {
        return std::unexpected("Failed to open shader file: " + path.string());
    }
    std::string contents;
    contents.assign(
        std::istreambuf_iterator<char>{file},
        std::istreambuf_iterator<char>{}
    );
    return contents;
}

inline std::expected<gpu::gl::ShaderId, std::string> compile_shader(
    gpu::gl::ShaderType type,
    std::string_view source
) {
    auto shader = gpu::gl::create_shader(type);
    if (shader == 0) {
        return std::unexpected(std::string{"glCreateShader failed"});
    }

    gpu::gl::set_shader_source(shader, source);

    if (!gpu::gl::compile_shader(shader)) {
        auto log = gpu::gl::shader_info_log(shader);
        if (log.empty()) {
            log = "Shader compilation failed without info log";
        }
        gpu::gl::delete_shader(shader);
        return std::unexpected(std::move(log));
    }

    return shader;
}

} // namespace detail

class Shader {
public:
    Shader() = default;

    [[nodiscard]] static std::expected<Shader, std::string> from_source(
        std::string_view vertex_src,
        std::string_view fragment_src
    ) {
        auto vertex = detail::compile_shader(
            gpu::gl::ShaderType::vertex,
            vertex_src
        );
        if (!vertex) return std::unexpected(vertex.error());

        auto fragment = detail::compile_shader(
            gpu::gl::ShaderType::fragment,
            fragment_src
        );
        if (!fragment) {
            if (*vertex != 0) gpu::gl::delete_shader(*vertex);
            return std::unexpected(fragment.error());
        }

        gpu::gl::ProgramId program = gpu::gl::create_program();
        if (program == 0) {
            gpu::gl::delete_shader(*vertex);
            gpu::gl::delete_shader(*fragment);
            return std::unexpected(std::string{"glCreateProgram failed"});
        }

        gpu::gl::attach_shader(program, *vertex);
        gpu::gl::attach_shader(program, *fragment);

        gpu::gl::delete_shader(*vertex);
        gpu::gl::delete_shader(*fragment);

        if (!gpu::gl::link_program(program)) {
            auto log = gpu::gl::program_info_log(program);
            if (log.empty()) {
                log = "Program link failed without info log";
            }
            gpu::gl::delete_program(program);
            return std::unexpected(std::move(log));
        }

        Shader shader;
        shader.id_ = program;
        return shader;
    }

    [[nodiscard]] static std::expected<Shader, std::string> from_files(
        const std::filesystem::path& vertex_path,
        const std::filesystem::path& fragment_path
    ) {
        auto vertex_src = detail::load_text_file(vertex_path);
        if (!vertex_src) return std::unexpected(vertex_src.error());

        auto fragment_src = detail::load_text_file(fragment_path);
        if (!fragment_src) return std::unexpected(fragment_src.error());

        return from_source(*vertex_src, *fragment_src);
    }

    ~Shader() {
        gpu::gl::delete_program(id_);
    }

    Shader(const Shader&)            = delete;
    Shader& operator=(const Shader&) = delete;

    Shader(Shader&& other) noexcept {
        std::swap(id_, other.id_);
    }

    Shader& operator=(Shader&& other) noexcept {
        if (this != &other) {
            gpu::gl::delete_program(id_);
            std::swap(id_, other.id_);
        }
        return *this;
    }

    void use() const {
        gpu::gl::use_program(id_);
    }

    bool set_float(std::string_view name, float value) const noexcept {
        if (auto loc = locate_uniform(name); loc != -1) {
            gpu::gl::set_uniform(loc, value);
            return true;
        }
        return false;
    }

    bool set_int(std::string_view name, int value) const noexcept {
        if (auto loc = locate_uniform(name); loc != -1) {
            gpu::gl::set_uniform(loc, value);
            return true;
        }
        return false;
    }

    bool set_vec3(std::string_view name, const core::Vec3& value) const noexcept {
        if (auto loc = locate_uniform(name); loc != -1) {
            gpu::gl::set_uniform_vec3(loc, value_ptr(value));
            return true;
        }
        return false;
    }

    bool set_mat3(std::string_view name, const core::Mat3& value) const noexcept {
        if (auto loc = locate_uniform(name); loc != -1) {
            gpu::gl::set_uniform_mat3(loc, value_ptr(value));
            return true;
        }
        return false;
    }

    bool set_mat4(std::string_view name, const core::Mat4& value) const noexcept {
        if (auto loc = locate_uniform(name); loc != -1) {
            gpu::gl::set_uniform_mat4(loc, value_ptr(value));
            return true;
        }
        return false;
    }

    [[nodiscard]] gpu::gl::ProgramId id() const noexcept { return id_; }

private:
    [[nodiscard]] gpu::gl::UniformLocation locate_uniform(std::string_view name) const {
        const std::string key{name};
        const auto it = uniform_cache_.find(key);
        if (it != uniform_cache_.end()) {
            return it->second;
        }
        const auto loc = gpu::gl::get_uniform_location(id_, key);
        uniform_cache_.emplace(key, loc);
        return loc;
    }

    gpu::gl::ProgramId id_{0};
    mutable std::unordered_map<std::string, gpu::gl::UniformLocation> uniform_cache_{};
};

} // namespace render
