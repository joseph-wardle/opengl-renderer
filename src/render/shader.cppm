export module render.shader;

import std;
import gpu.gl;

export namespace render {

namespace detail {

inline gpu::gl::ShaderId compile_shader(
    gpu::gl::ShaderType type,
    std::string_view source
) {
    auto shader = gpu::gl::create_shader(type);
    gpu::gl::set_shader_source(shader, source);

    if (!gpu::gl::compile_shader(shader)) {
        const auto log = gpu::gl::shader_info_log(shader);
        std::cerr << "Shader compilation failed: " << log << '\n';
        gpu::gl::delete_shader(shader);
        return 0;
    }

    return shader;
}

} // namespace detail

class Shader {
public:
    Shader() = default;

    static std::optional<Shader> from_source(
        std::string_view vertex_src,
        std::string_view fragment_src
    ) {
        auto vertex = detail::compile_shader(
            gpu::gl::ShaderType::vertex,
            vertex_src
        );
        if (vertex == 0) return std::nullopt;

        auto fragment = detail::compile_shader(
            gpu::gl::ShaderType::fragment,
            fragment_src
        );
        if (fragment == 0) {
            gpu::gl::delete_shader(vertex);
            return std::nullopt;
        }

        gpu::gl::ProgramId program = gpu::gl::create_program();
        gpu::gl::attach_shader(program, vertex);
        gpu::gl::attach_shader(program, fragment);

        gpu::gl::delete_shader(vertex);
        gpu::gl::delete_shader(fragment);

        if (!gpu::gl::link_program(program)) {
            const auto log = gpu::gl::program_info_log(program);
            std::cerr << "Program link failed: " << log << '\n';
            gpu::gl::delete_program(program);
            return std::nullopt;
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
