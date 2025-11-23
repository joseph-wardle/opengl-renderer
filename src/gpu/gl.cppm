module;

#include <glad/gl.h>

export module gpu.gl;

export namespace gpu::gl {

constexpr unsigned COLOR_BUFFER_BIT = GL_COLOR_BUFFER_BIT;
constexpr unsigned DEPTH_BUFFER_BIT = GL_DEPTH_BUFFER_BIT;

using LoadFunc = GLADloadfunc;

[[nodiscard]] inline bool init(LoadFunc load) noexcept {
    if (!load) {
        return false;
    }
    return gladLoadGL(load) != 0;
}

inline void viewport(int x, int y, int w, int h) {
    ::glViewport(x, y, w, h);
}

inline void clear(unsigned mask) {
    ::glClear(mask);
}

inline void clear_color(float r, float g, float b, float a) {
    ::glClearColor(r, g, b, a);
}

} // namespace gpu::gl
