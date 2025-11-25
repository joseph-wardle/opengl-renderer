module;

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

export module resources.image;

import std;

export namespace resources {

struct ImageData {
    int width{0};
    int height{0};
    int channels{0};  // e.g., 1, 3, 4
    std::vector<std::uint8_t> pixels;  // tightly packed
};

inline std::expected<ImageData, std::string>
load_image(const std::string& path, bool flip_vertically = true) {
    stbi_set_flip_vertically_on_load(flip_vertically ? 1 : 0);

    int w = 0;
    int h = 0;
    int c = 0;
    unsigned char* data = stbi_load(path.c_str(), &w, &h, &c, 0);

    if (!data) {
        const char* reason = stbi_failure_reason();
        return std::unexpected(
            reason ? std::string{"stb_image: "} + reason : std::string{"stb_image: unknown failure"}
        );
    }

    ImageData img;
    img.width    = w;
    img.height   = h;
    img.channels = c;
    img.pixels.assign(data, data + (w * h * c));

    stbi_image_free(data);
    return img;
}

} // namespace resources

