export module render.shader;

import std;
import gpu.gl;

export namespace render {

namespace detail {

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

    [[nodiscard]] gpu::gl::ProgramId id() const noexcept { return id_; }

private:
    gpu::gl::ProgramId id_{0};
};

} // namespace render
