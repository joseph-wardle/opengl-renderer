export module render.mesh;

import std;
import core.glm;
import gpu.gl;
import render.vertex_array;
import render.buffer;

export namespace render {

struct Vertex {
    core::Vec3 position{};
    core::Vec3 normal{};
    core::Vec2 uv{};
};

class Mesh {
public:
    Mesh() = default;

    [[nodiscard]] static std::expected<Mesh, std::string> from_data(
        std::span<const Vertex> vertices,
        std::span<const std::uint32_t> indices
    ) {
        Mesh mesh;

        auto vbo = VertexBuffer::from_data(vertices.data(), vertices.size_bytes());
        if (!vbo) {
            return std::unexpected(std::string{"Vertex buffer creation failed: "} + vbo.error());
        }
        mesh.vbo_ = std::move(*vbo);

        auto ibo = IndexBuffer::from_data(indices.data(), indices.size_bytes());
        if (!ibo) {
            return std::unexpected(std::string{"Index buffer creation failed: "} + ibo.error());
        }
        mesh.ibo_ = std::move(*ibo);

        auto vao = VertexArray::create();
        if (!vao) {
            return std::unexpected(std::string{"Vertex array creation failed: "} + vao.error());
        }
        mesh.vao_ = std::move(*vao);

        mesh.vao_.bind();
        mesh.vbo_.bind();
        mesh.ibo_.bind();

        const int stride = static_cast<int>(sizeof(Vertex));
        mesh.vao_.set_attribute_float(0, 3, stride, 0);
        mesh.vao_.set_attribute_float(1, 3, stride, sizeof(core::Vec3));
        mesh.vao_.set_attribute_float(2, 2, stride, sizeof(core::Vec3) * 2);

        VertexArray::unbind();
        VertexBuffer::unbind();
        IndexBuffer::unbind();

        mesh.index_count_ = static_cast<int>(indices.size());
        return mesh;
    }

    void bind() const { vao_.bind(); }
    void unbind() const { VertexArray::unbind(); }

    void draw() const {
        vao_.bind();
        ibo_.bind();
        gpu::gl::draw_elements(gpu::gl::Primitive::triangles, index_count_, gpu::gl::IndexType::u32);
        IndexBuffer::unbind();
        VertexArray::unbind();
    }

    [[nodiscard]] bool is_valid() const noexcept {
        return vao_.is_valid() && vbo_.is_valid() && ibo_.is_valid() && index_count_ > 0;
    }

private:
    VertexArray  vao_{};
    VertexBuffer vbo_{};
    IndexBuffer  ibo_{};
    int          index_count_{0};
};

} // namespace render
