export module render.vertex_array;

import std;
import gpu.gl;

export namespace render {

class VertexArray {
public:
    VertexArray() = default;

    [[nodiscard]] static std::expected<VertexArray, std::string> create() {
        VertexArray vao;
        vao.id_ = gpu::gl::create_vertex_array();
        if (vao.id_ == 0) {
            return std::unexpected(std::string{"glGenVertexArrays failed"});
        }
        return vao;
    }

    ~VertexArray() {
        gpu::gl::destroy_vertex_array(id_);
    }

    VertexArray(const VertexArray&) = delete;
    VertexArray& operator=(const VertexArray&) = delete;

    VertexArray(VertexArray&& other) noexcept {
        std::swap(id_, other.id_);
    }

    VertexArray& operator=(VertexArray&& other) noexcept {
        if (this != &other) {
            gpu::gl::destroy_vertex_array(id_);
            std::swap(id_, other.id_);
        }
        return *this;
    }

    void bind() const {
        gpu::gl::bind_vertex_array(id_);
    }

    static void unbind() {
        gpu::gl::bind_vertex_array(0);
    }

    // Simple helper for float attributes (e.g. vec3 position)
    void set_attribute_float(
        unsigned int index,
        int component_count,
        int stride_bytes,
        std::size_t offset_bytes
    ) const {
        gpu::gl::set_vertex_attrib_pointer(
            index,
            component_count,
            gpu::gl::VertexAttribType::f32,
            false,
            stride_bytes,
            offset_bytes
        );
        gpu::gl::enable_vertex_attrib_array(index);
    }

    [[nodiscard]] bool is_valid() const noexcept { return id_ != 0; }

private:
    gpu::gl::VertexArrayId id_{0};
};

} // namespace render
