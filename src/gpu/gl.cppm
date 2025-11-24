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
};

[[nodiscard]] inline bool init(LoadProc load_proc) {
    return load_proc && gladLoadGL(load_proc) != 0;
}

inline void viewport(int x, int y, int width, int height) {
    glViewport(x, y, width, height);
}

inline void clear_color(float r, float g, float b, float a) {
    glClearColor(r, g, b, a);
}

inline void clear(ClearMask mask) {
    glClear(mask);
}

[[nodiscard]] inline BufferId create_buffer() {
    BufferId id{};
    glGenBuffers(1, &id);
    return id;
}

inline void destroy_buffer(BufferId& id) {
    if (id != 0) {
        glDeleteBuffers(1, &id);
        id = 0;
    }
}

inline void bind_buffer(BufferTarget target, BufferId id) {
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

[[nodiscard]] inline VertexArrayId create_vertex_array() {
    VertexArrayId id{};
    glGenVertexArrays(1, &id);
    return id;
}

inline void destroy_vertex_array(VertexArrayId& id) {
    if (id != 0) {
        glDeleteVertexArrays(1, &id);
        id = 0;
    }
}

inline void bind_vertex_array(VertexArrayId id) {
    glBindVertexArray(id);
}

inline void set_vertex_attrib_pointer(
    unsigned int index,
    int component_count,
    VertexAttribType type,
    bool normalized,
    int stride_bytes,
    std::size_t offset_bytes
) {
    glVertexAttribPointer(
        index,
        component_count,
        static_cast<GLenum>(type),
        normalized ? GL_TRUE : GL_FALSE,
        stride_bytes,
        reinterpret_cast<const void*>(offset_bytes)
    );
}

inline void enable_vertex_attrib_array(unsigned int index) {
    glEnableVertexAttribArray(index);
}

inline ShaderId create_shader(ShaderType type) {
    return glCreateShader(static_cast<GLenum>(type));
}

inline void set_shader_source(ShaderId shader, std::string_view source) {
    const char* src = source.data();
    glShaderSource(shader, 1, &src, nullptr);
}

inline bool compile_shader(ShaderId shader) {
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

inline void delete_shader(ShaderId& shader) {
    if (shader != 0) {
        glDeleteShader(shader);
        shader = 0;
    }
}

inline ProgramId create_program() {
    return glCreateProgram();
}

inline void attach_shader(ProgramId program, ShaderId shader) {
    glAttachShader(program, shader);
}

inline bool link_program(ProgramId program) {
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

inline void delete_program(ProgramId& program) {
    if (program != 0) {
        glDeleteProgram(program);
        program = 0;
    }
}

inline void use_program(ProgramId program) {
    glUseProgram(program);
}

inline void draw_arrays(Primitive primitive, int first, int count) {
    glDrawArrays(static_cast<GLenum>(primitive), first, count);
}

inline void draw_elements(Primitive primitive, int count, IndexType type, std::size_t offset_bytes = 0) {
    glDrawElements(
        static_cast<GLenum>(primitive),
        count,
        static_cast<GLenum>(type),
        reinterpret_cast<const void*>(offset_bytes)
    );
}

inline void polygon_mode(Face face, PolygonMode mode) {
    glPolygonMode(static_cast<GLenum>(face), static_cast<GLenum>(mode));
}

} // namespace gpu::gl
