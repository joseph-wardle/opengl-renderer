export module render.context;

import gpu.gl;

export namespace render {

struct FrameClear {
    float r{0.0f};
    float g{0.0f};
    float b{0.0f};
    float a{1.0f};
};

class Context {
public:
    void begin_frame(const FrameClear& clear) const noexcept {
        gpu::gl::clear_color(clear.r, clear.g, clear.b, clear.a);
        gpu::gl::clear(gpu::gl::COLOR_BUFFER_BIT | gpu::gl::DEPTH_BUFFER_BIT);
    }

    void set_viewport(int width, int height) const noexcept {
        gpu::gl::viewport(0, 0, width, height);
    }

    void set_wireframe(bool enabled) const noexcept {
        const auto mode = enabled
            ? gpu::gl::PolygonMode::line
            : gpu::gl::PolygonMode::fill;
        gpu::gl::polygon_mode(gpu::gl::Face::front_and_back, mode);
    }

    void set_depth_test(bool enabled) const noexcept {
        gpu::gl::enable_depth_test(enabled);
    }

    void set_alpha_blending(bool enabled) const noexcept {
        gpu::gl::enable_blend(enabled);
        if (enabled) {
            gpu::gl::blend_func(
                gpu::gl::BlendFactor::src_alpha,
                gpu::gl::BlendFactor::one_minus_src_alpha
            );
        }
    }

    void set_depth_write(bool enabled) const noexcept {
        gpu::gl::depth_mask(enabled);
    }

    void set_backface_culling(bool enabled, gpu::gl::Face culled_face = gpu::gl::Face::back) const noexcept {
        gpu::gl::enable_cull_face(enabled);
        if (enabled) {
            gpu::gl::cull_face(culled_face);
        }
    }
};

} // namespace render
