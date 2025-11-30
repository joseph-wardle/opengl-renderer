module;

#include <glad/gl.h>

export module gpu.gl;

import std;

export namespace gpu::gl {

using LoadProc      = GLADloadfunc;
using ClearMask     = unsigned int;
using BufferId      = GLuint;
using VertexArrayId = GLuint;
using ShaderId      = GLuint;
using ProgramId     = GLuint;
using TextureId     = GLuint;
using UniformLocation = GLint;

inline bool g_initialized = false;

enum class BufferTarget : GLenum {
    array = GL_ARRAY_BUFFER,
    element_array = GL_ELEMENT_ARRAY_BUFFER,
};

enum class BufferUsage : GLenum {
    stream_draw  = GL_STREAM_DRAW,
    static_draw  = GL_STATIC_DRAW,
    dynamic_draw = GL_DYNAMIC_DRAW,
};

enum class VertexAttribType : GLenum {
    f32 = GL_FLOAT,
};

enum class ShaderType : GLenum {
    vertex   = GL_VERTEX_SHADER,
    fragment = GL_FRAGMENT_SHADER,
};

enum class Primitive : GLenum {
    triangles = GL_TRIANGLES,
};

enum class IndexType : GLenum {
    u16 = GL_UNSIGNED_SHORT,
    u32 = GL_UNSIGNED_INT,
};

enum class PolygonMode : GLenum {
    fill = GL_FILL,
    line = GL_LINE,
};

enum class Face : GLenum {
    front_and_back = GL_FRONT_AND_BACK,
    front = GL_FRONT,
    back  = GL_BACK,
};

enum class BlendFactor : GLenum {
    zero = GL_ZERO,
    one = GL_ONE,
    src_alpha = GL_SRC_ALPHA,
    one_minus_src_alpha = GL_ONE_MINUS_SRC_ALPHA,
};

enum class TextureTarget : GLenum {
    texture_2d = GL_TEXTURE_2D,
};

enum class TextureParam : GLenum {
    wrap_s    = GL_TEXTURE_WRAP_S,
    wrap_t    = GL_TEXTURE_WRAP_T,
    min_filter = GL_TEXTURE_MIN_FILTER,
    mag_filter = GL_TEXTURE_MAG_FILTER,
};

enum class TextureWrap : GLenum {
    repeat       = GL_REPEAT,
    clamp_to_edge = GL_CLAMP_TO_EDGE,
};

enum class TextureFilter : GLenum {
    nearest              = GL_NEAREST,
    linear               = GL_LINEAR,
    linear_mipmap_linear = GL_LINEAR_MIPMAP_LINEAR,
};

enum class PixelFormat : GLenum {
    red  = GL_RED,
    rgb  = GL_RGB,
    rgba = GL_RGBA,
};

enum class PixelType : GLenum {
    u8 = GL_UNSIGNED_BYTE,
};

[[nodiscard]] inline bool init(LoadProc load_proc) noexcept {
    g_initialized = load_proc && gladLoadGL(load_proc) != 0;
    return g_initialized;
}

[[nodiscard]] inline bool is_initialized() noexcept { return g_initialized; }

inline void shutdown() noexcept { g_initialized = false; }

inline void viewport(int x, int y, int width, int height) noexcept {
    glViewport(x, y, width, height);
}

inline void clear_color(float r, float g, float b, float a) noexcept {
    glClearColor(r, g, b, a);
}

inline void clear(ClearMask mask) noexcept {
    glClear(mask);
}

inline void enable_depth_test(bool enable) noexcept {
    if (enable) {
        glEnable(GL_DEPTH_TEST);
    } else {
        glDisable(GL_DEPTH_TEST);
    }
}

inline void enable_blend(bool enable) noexcept {
    if (enable) {
        glEnable(GL_BLEND);
    } else {
        glDisable(GL_BLEND);
    }
}

inline void blend_func(BlendFactor src_factor, BlendFactor dst_factor) noexcept {
    glBlendFunc(static_cast<GLenum>(src_factor), static_cast<GLenum>(dst_factor));
}

inline void depth_mask(bool enable) noexcept {
    glDepthMask(enable ? GL_TRUE : GL_FALSE);
}

inline void enable_cull_face(bool enable) noexcept {
    if (enable) {
        glEnable(GL_CULL_FACE);
    } else {
        glDisable(GL_CULL_FACE);
    }
}

inline void cull_face(Face face) noexcept {
    glCullFace(static_cast<GLenum>(face));
}

[[nodiscard]] inline BufferId create_buffer() noexcept {
    BufferId id{};
    glGenBuffers(1, &id);
    return id;
}

inline void destroy_buffer(BufferId& id) noexcept {
    if (!g_initialized) {
        id = 0;
        return;
    }
    if (id != 0) {
        glDeleteBuffers(1, &id);
        id = 0;
    }
}

inline void bind_buffer(BufferTarget target, BufferId id) noexcept {
    glBindBuffer(static_cast<GLenum>(target), id);
}

inline void buffer_data(
    BufferTarget target,
    std::size_t byte_size,
    const void* data,
    BufferUsage usage
) {
    glBufferData(
        static_cast<GLenum>(target),
        static_cast<GLsizeiptr>(byte_size),
        data,
        static_cast<GLenum>(usage)
    );
}

constexpr ClearMask COLOR_BUFFER_BIT = GL_COLOR_BUFFER_BIT;
constexpr ClearMask DEPTH_BUFFER_BIT = GL_DEPTH_BUFFER_BIT;

[[nodiscard]] inline VertexArrayId create_vertex_array() noexcept {
    VertexArrayId id{};
    glGenVertexArrays(1, &id);
    return id;
}

inline void destroy_vertex_array(VertexArrayId& id) noexcept {
    if (!g_initialized) {
        id = 0;
        return;
    }
    if (id != 0) {
        glDeleteVertexArrays(1, &id);
        id = 0;
    }
}

inline void bind_vertex_array(VertexArrayId id) noexcept {
    glBindVertexArray(id);
}

inline void set_vertex_attrib_pointer(
    unsigned int index,
    int component_count,
    VertexAttribType type,
    bool normalized,
    int stride_bytes,
    std::size_t offset_bytes
) noexcept {
    glVertexAttribPointer(
        index,
        component_count,
        static_cast<GLenum>(type),
        normalized ? GL_TRUE : GL_FALSE,
        stride_bytes,
        reinterpret_cast<const void*>(offset_bytes)
    );
}

inline void enable_vertex_attrib_array(unsigned int index) noexcept {
    glEnableVertexAttribArray(index);
}

inline ShaderId create_shader(ShaderType type) noexcept {
    return glCreateShader(static_cast<GLenum>(type));
}

inline void set_shader_source(ShaderId shader, std::string_view source) noexcept {
    const char* src = source.data();
    glShaderSource(shader, 1, &src, nullptr);
}

inline bool compile_shader(ShaderId shader) noexcept {
    glCompileShader(shader);
    GLint status = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    return status == GL_TRUE;
}

inline std::string shader_info_log(ShaderId shader) {
    GLint length = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
    if (length <= 1) return {};

    std::string log(static_cast<std::size_t>(length), '\0');
    GLsizei written = 0;
    glGetShaderInfoLog(shader, length, &written, log.data());
    log.resize(static_cast<std::size_t>(written));
    return log;
}

inline void delete_shader(ShaderId& shader) noexcept {
    if (shader != 0) {
        glDeleteShader(shader);
        shader = 0;
    }
}

inline ProgramId create_program() noexcept {
    return glCreateProgram();
}

inline void attach_shader(ProgramId program, ShaderId shader) noexcept {
    glAttachShader(program, shader);
}

inline bool link_program(ProgramId program) noexcept {
    glLinkProgram(program);
    GLint status = GL_FALSE;
    glGetProgramiv(program, GL_LINK_STATUS, &status);
    return status == GL_TRUE;
}

inline std::string program_info_log(ProgramId program) {
    GLint length = 0;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
    if (length <= 1) return {};

    std::string log(static_cast<std::size_t>(length), '\0');
    GLsizei written = 0;
    glGetProgramInfoLog(program, length, &written, log.data());
    log.resize(static_cast<std::size_t>(written));
    return log;
}

inline void delete_program(ProgramId& program) noexcept {
    if (!g_initialized) {
        program = 0;
        return;
    }
    if (program != 0) {
        glDeleteProgram(program);
        program = 0;
    }
}

inline void use_program(ProgramId program) noexcept {
    glUseProgram(program);
}

inline void draw_arrays(Primitive primitive, int first, int count) noexcept {
    glDrawArrays(static_cast<GLenum>(primitive), first, count);
}

inline void draw_elements(Primitive primitive, int count, IndexType type, std::size_t offset_bytes = 0) noexcept {
    glDrawElements(
        static_cast<GLenum>(primitive),
        count,
        static_cast<GLenum>(type),
        reinterpret_cast<const void*>(offset_bytes)
    );
}

inline void polygon_mode(Face face, PolygonMode mode) noexcept {
    glPolygonMode(static_cast<GLenum>(face), static_cast<GLenum>(mode));
}

inline UniformLocation get_uniform_location(ProgramId program, std::string_view name) noexcept {
    const std::string owned{name};
    return glGetUniformLocation(program, owned.c_str());
}

inline void set_uniform(UniformLocation location, float v0) noexcept {
    glUniform1f(location, v0);
}

inline void set_uniform(UniformLocation location, int v0) noexcept {
    glUniform1i(location, v0);
}

inline void set_uniform_mat4(UniformLocation location, const float* data) noexcept {
    glUniformMatrix4fv(location, 1, GL_FALSE, data);
}

inline void set_uniform_mat3(UniformLocation location, const float* data) noexcept {
    glUniformMatrix3fv(location, 1, GL_FALSE, data);
}

inline void set_uniform_vec3(UniformLocation location, const float* data) noexcept {
    glUniform3fv(location, 1, data);
}

[[nodiscard]] inline TextureId create_texture() noexcept {
    TextureId id{};
    glGenTextures(1, &id);
    return id;
}

inline void destroy_texture(TextureId& id) noexcept {
    if (!g_initialized) {
        id = 0;
        return;
    }
    if (id != 0) {
        glDeleteTextures(1, &id);
        id = 0;
    }
}

inline void active_texture(unsigned int unit) noexcept {
    glActiveTexture(GL_TEXTURE0 + unit);
}

inline void bind_texture(TextureTarget target, TextureId id) {
    glBindTexture(static_cast<GLenum>(target), id);
}

inline void tex_image_2d(
    TextureTarget target,
    int level,
    PixelFormat internal_format,
    int width,
    int height,
    PixelFormat format,
    PixelType type,
    const void* data
) {
    glTexImage2D(
        static_cast<GLenum>(target),
        level,
        static_cast<GLint>(internal_format),
        width,
        height,
        0,
        static_cast<GLenum>(format),
        static_cast<GLenum>(type),
        data
    );
}

inline void generate_mipmap(TextureTarget target) {
    glGenerateMipmap(static_cast<GLenum>(target));
}

inline void set_texture_parameter(
    TextureTarget target,
    TextureParam param,
    int value
) {
    glTexParameteri(
        static_cast<GLenum>(target),
        static_cast<GLenum>(param),
        value
    );
}

} // namespace gpu::gl
