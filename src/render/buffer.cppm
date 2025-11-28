export module render.buffer;

import std;
import gpu.gl;

export namespace render {

class VertexBuffer {
public:
    VertexBuffer() = default;

    [[nodiscard]] static std::expected<VertexBuffer, std::string>
    from_data(const void* data, std::size_t byte_size) {
        VertexBuffer vb;
        vb.id_ = gpu::gl::create_buffer();
        if (vb.id_ == 0) {
            return std::unexpected(std::string{"glGenBuffers failed"});
        }
        vb.bind();
        gpu::gl::buffer_data(
            gpu::gl::BufferTarget::array,
            byte_size,
            data,
            gpu::gl::BufferUsage::static_draw
        );
        return vb;
    }

    ~VertexBuffer() {
        gpu::gl::destroy_buffer(id_);
    }

    VertexBuffer(const VertexBuffer&) = delete;
    VertexBuffer& operator=(const VertexBuffer&) = delete;

    VertexBuffer(VertexBuffer&& other) noexcept {
        std::swap(id_, other.id_);
    }

    VertexBuffer& operator=(VertexBuffer&& other) noexcept {
        if (this != &other) {
            gpu::gl::destroy_buffer(id_);
            std::swap(id_, other.id_);
        }
        return *this;
    }

    void bind() const noexcept {
        gpu::gl::bind_buffer(gpu::gl::BufferTarget::array, id_);
    }

    static void unbind() noexcept {
        gpu::gl::bind_buffer(gpu::gl::BufferTarget::array, 0);
    }

    [[nodiscard]] bool is_valid() const noexcept { return id_ != 0; }

private:
    gpu::gl::BufferId id_{0};
};

class IndexBuffer {
public:
    IndexBuffer() = default;

    [[nodiscard]] static std::expected<IndexBuffer, std::string>
    from_data(const void* data, std::size_t byte_size) {
        IndexBuffer ibo;
        ibo.id_ = gpu::gl::create_buffer();
        if (ibo.id_ == 0) {
            return std::unexpected(std::string{"glGenBuffers failed"});
        }
        ibo.bind();
        gpu::gl::buffer_data(
            gpu::gl::BufferTarget::element_array,
            byte_size,
            data,
            gpu::gl::BufferUsage::static_draw
        );
        return ibo;
    }

    ~IndexBuffer() {
        gpu::gl::destroy_buffer(id_);
    }

    IndexBuffer(const IndexBuffer&) = delete;
    IndexBuffer& operator=(const IndexBuffer&) = delete;

    IndexBuffer(IndexBuffer&& other) noexcept {
        std::swap(id_, other.id_);
    }

    IndexBuffer& operator=(IndexBuffer&& other) noexcept {
        if (this != &other) {
            gpu::gl::destroy_buffer(id_);
            std::swap(id_, other.id_);
        }
        return *this;
    }

    void bind() const noexcept {
        gpu::gl::bind_buffer(gpu::gl::BufferTarget::element_array, id_);
    }

    static void unbind() noexcept {
        gpu::gl::bind_buffer(gpu::gl::BufferTarget::element_array, 0);
    }

    [[nodiscard]] bool is_valid() const noexcept { return id_ != 0; }

private:
    gpu::gl::BufferId id_{0};
};

} // namespace render
