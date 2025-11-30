export module render.texture;

import std;
import gpu.gl;
import resources.image;

export namespace render {

enum class TextureWrap {
    repeat,
    clamp_to_edge,
};

enum class TextureFilter {
    nearest,
    linear,
};

enum class AlphaMode {
    opaque,
    mask,   // binary cutout (alpha 0 or 1)
    blend,  // smooth/translucent alpha
};

class Texture2D {
public:
    Texture2D() = default;

    [[nodiscard]] static std::expected<Texture2D, std::string>
    from_image(const resources::ImageData& img, bool generate_mipmaps = true) {
        const auto format = to_pixel_format(img.channels);
        if (!format) {
            return std::unexpected(std::string{"Unsupported channel count"});
        }

        Texture2D tex;
        tex.id_ = gpu::gl::create_texture();
        if (tex.id_ == 0) {
            return std::unexpected(std::string{"glGenTextures failed"});
        }
        tex.alpha_mode_ = classify_alpha(img);
        tex.has_alpha_channel_ = img.channels == 4;
        tex.has_transparency_ = tex.alpha_mode_ != AlphaMode::opaque;

        gpu::gl::bind_texture(gpu::gl::TextureTarget::texture_2d, tex.id_);
        gpu::gl::tex_image_2d(
            gpu::gl::TextureTarget::texture_2d,
            0,
            *format,
            img.width,
            img.height,
            *format,
            gpu::gl::PixelType::u8,
            img.pixels.data()
        );

        if (generate_mipmaps) {
            gpu::gl::generate_mipmap(gpu::gl::TextureTarget::texture_2d);
            tex.has_mipmaps_ = true;
        }

        gpu::gl::set_texture_parameter(
            gpu::gl::TextureTarget::texture_2d,
            gpu::gl::TextureParam::wrap_s,
            static_cast<int>(gpu::gl::TextureWrap::repeat)
        );
        gpu::gl::set_texture_parameter(
            gpu::gl::TextureTarget::texture_2d,
            gpu::gl::TextureParam::wrap_t,
            static_cast<int>(gpu::gl::TextureWrap::repeat)
        );

        const auto min_filter =
            tex.has_mipmaps_
                ? gpu::gl::TextureFilter::linear_mipmap_linear
                : gpu::gl::TextureFilter::linear;

        gpu::gl::set_texture_parameter(
            gpu::gl::TextureTarget::texture_2d,
            gpu::gl::TextureParam::min_filter,
            static_cast<int>(min_filter)
        );
        gpu::gl::set_texture_parameter(
            gpu::gl::TextureTarget::texture_2d,
            gpu::gl::TextureParam::mag_filter,
            static_cast<int>(gpu::gl::TextureFilter::linear)
        );

        gpu::gl::bind_texture(gpu::gl::TextureTarget::texture_2d, 0);
        return tex;
    }

    ~Texture2D() {
        gpu::gl::destroy_texture(id_);
    }

    Texture2D(const Texture2D&)            = delete;
    Texture2D& operator=(const Texture2D&) = delete;

    Texture2D(Texture2D&& other) noexcept {
        std::swap(id_, other.id_);
        std::swap(has_mipmaps_, other.has_mipmaps_);
        std::swap(has_alpha_channel_, other.has_alpha_channel_);
        std::swap(has_transparency_, other.has_transparency_);
        std::swap(alpha_mode_, other.alpha_mode_);
    }

    Texture2D& operator=(Texture2D&& other) noexcept {
        if (this != &other) {
            gpu::gl::destroy_texture(id_);
            std::swap(id_, other.id_);
            std::swap(has_mipmaps_, other.has_mipmaps_);
            std::swap(has_alpha_channel_, other.has_alpha_channel_);
            std::swap(has_transparency_, other.has_transparency_);
            std::swap(alpha_mode_, other.alpha_mode_);
        }
        return *this;
    }

    void bind(unsigned int texture_unit = 0) const noexcept {
        gpu::gl::active_texture(texture_unit);
        gpu::gl::bind_texture(gpu::gl::TextureTarget::texture_2d, id_);
    }

    static void unbind() noexcept {
        gpu::gl::bind_texture(gpu::gl::TextureTarget::texture_2d, 0);
    }

    [[nodiscard]] bool is_valid() const noexcept { return id_ != 0; }
    [[nodiscard]] gpu::gl::TextureId id() const noexcept { return id_; }
    [[nodiscard]] bool has_alpha_channel() const noexcept { return has_alpha_channel_; }
    [[nodiscard]] bool has_transparency() const noexcept { return has_transparency_; }
    [[nodiscard]] AlphaMode alpha_mode() const noexcept { return alpha_mode_; }

private:
    static std::optional<gpu::gl::PixelFormat> to_pixel_format(int channels) {
        switch (channels) {
        case 1: return gpu::gl::PixelFormat::red;
        case 3: return gpu::gl::PixelFormat::rgb;
        case 4: return gpu::gl::PixelFormat::rgba;
        default: return std::nullopt;
        }
    }

    static AlphaMode classify_alpha(const resources::ImageData& img) {
        if (img.channels < 4) {
            return AlphaMode::opaque;
        }
        const auto stride = static_cast<std::size_t>(img.channels);
        bool has_zero = false;
        for (std::size_t i = 0; i + 3 < img.pixels.size(); i += stride) {
            const auto a = img.pixels[i + 3];
            if (a == 0) {
                has_zero = true;
            } else if (a < 255) {
                return AlphaMode::blend;
            }
        }
        return has_zero ? AlphaMode::mask : AlphaMode::opaque;
    }

    gpu::gl::TextureId id_{0};
    bool               has_mipmaps_{false};
    bool               has_alpha_channel_{false};
    bool               has_transparency_{false};
    AlphaMode          alpha_mode_{AlphaMode::opaque};
};

} // namespace render
