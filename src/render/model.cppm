export module render.model;

import std;
import core.glm;
import render.mesh;
import render.tinyobj;

export namespace render {

struct Model {
    struct Part {
        Mesh        mesh;
        core::Vec3  base_color{1.0f, 1.0f, 1.0f};
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
        model.parts.push_back(Model::Part{
            .mesh = std::move(*mesh),
            .base_color = color,
        });
    }

    return model;
}

} // namespace render
