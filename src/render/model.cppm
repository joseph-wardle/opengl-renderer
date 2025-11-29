export module render.model;

import std;
import core.glm;
import render.mesh;
import render.tinyobj;
import render.texture;
import resources.image;

export namespace render {

struct Model {
    struct Part {
        Mesh                           mesh;
        core::Vec3                     base_color{1.0f, 1.0f, 1.0f};
        std::shared_ptr<Texture2D>     diffuse_map{};
        std::shared_ptr<Texture2D>     specular_map{};

        [[nodiscard]] bool has_diffuse() const noexcept { return static_cast<bool>(diffuse_map); }
        [[nodiscard]] bool has_specular() const noexcept { return static_cast<bool>(specular_map); }
    };
    std::vector<Part> parts;
};

struct ModelLoadError {
    std::string message;
};

inline std::expected<Model, ModelLoadError>
load_obj_model(const std::filesystem::path& path) {
    auto obj = tinyobj::load_obj(path);
    if (!obj) {
        return std::unexpected(ModelLoadError{obj.error().message});
    }

    Model model{};
    model.parts.reserve(obj->meshes.size());

    const auto base_dir = path.parent_path();
    std::unordered_map<std::string, std::shared_ptr<Texture2D>> texture_cache{};

    for (const auto& src : obj->meshes) {
        const bool has_normals   = src.normals.size() == src.positions.size();
        const bool has_texcoords = src.texcoords.size() / 2 == src.positions.size() / 3;

        std::vector<Vertex> vertices;
        vertices.reserve(src.positions.size() / 3);
        std::vector<std::uint32_t> indices;
        indices.reserve(src.indices.size());

        for (std::size_t i = 0; i < src.positions.size(); i += 3) {
            Vertex v{};
            v.position = core::Vec3{
                src.positions[i + 0],
                src.positions[i + 1],
                src.positions[i + 2]
            };
            if (has_normals) {
                v.normal = core::Vec3{
                    src.normals[i + 0],
                    src.normals[i + 1],
                    src.normals[i + 2]
                };
            } else {
                v.normal = core::Vec3{0.0f, 1.0f, 0.0f};
            }
            if (has_texcoords) {
                const std::size_t ti = (i / 3) * 2;
                v.uv = core::Vec2{
                    src.texcoords[ti + 0],
                    src.texcoords[ti + 1]
                };
            } else {
                v.uv = core::Vec2{0.0f};
            }
            vertices.push_back(v);
        }

        indices.insert(indices.end(), src.indices.begin(), src.indices.end());

        if (!has_normals) {
            // Compute per-vertex normals from positions/indices.
            std::vector<core::Vec3> accum(vertices.size(), core::Vec3{0.0f});
            for (std::size_t i = 0; i + 2 < indices.size(); i += 3) {
                const auto i0 = indices[i + 0];
                const auto i1 = indices[i + 1];
                const auto i2 = indices[i + 2];
                const auto& v0 = vertices[i0].position;
                const auto& v1 = vertices[i1].position;
                const auto& v2 = vertices[i2].position;
                const auto n = normalize(cross(v1 - v0, v2 - v0));
                accum[i0] += n;
                accum[i1] += n;
                accum[i2] += n;
            }
            for (std::size_t i = 0; i < vertices.size(); ++i) {
                vertices[i].normal = normalize(accum[i]);
            }
        }

        auto mesh = Mesh::from_data(vertices, indices);
        if (!mesh) {
            return std::unexpected(ModelLoadError{mesh.error()});
        }
        const core::Vec3 color{
            src.diffuse[0],
            src.diffuse[1],
            src.diffuse[2],
        };

        Model::Part part;
        part.mesh = std::move(*mesh);
        part.base_color = color;

        auto load_texture = [&](const std::string& texname, std::shared_ptr<Texture2D>& out_tex) {
            if (texname.empty()) return;
            if (auto cached = texture_cache.find(texname); cached != texture_cache.end()) {
                out_tex = cached->second;
                return;
            }
            std::string clean = texname;
            std::replace(clean.begin(), clean.end(), '\\', '/');
            auto tex_path = base_dir / clean;
            auto img = resources::load_image(tex_path.string());
            if (!img) {
                std::println(std::cerr, "Failed to load texture {}: {}", tex_path.string(), img.error());
                return;
            }
            auto tex = Texture2D::from_image(*img, true);
            if (!tex) {
                std::println(std::cerr, "Failed to create texture {}: {}", tex_path.string(), tex.error());
                return;
            }
            out_tex = std::make_shared<Texture2D>(std::move(*tex));
            texture_cache.emplace(texname, out_tex);
        };

        load_texture(src.diffuse_tex, part.diffuse_map);
        load_texture(src.specular_tex, part.specular_map);

        model.parts.push_back(std::move(part));
    }

    return model;
}

} // namespace render
