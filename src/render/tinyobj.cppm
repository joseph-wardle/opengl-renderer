module;

#include <tiny_obj_loader.h>

export module render.tinyobj;

import std;

export namespace render::tinyobj {

struct ObjMesh {
    std::vector<float> positions; // xyz triplets
    std::vector<float> normals;   // xyz triplets (optional)
    std::vector<float> texcoords; // uv pairs (optional)
    std::vector<unsigned int> indices;
    std::array<float, 3> diffuse{1.0f, 1.0f, 1.0f};
    std::string diffuse_tex;
    std::string specular_tex;
};

struct ObjModel {
    std::vector<ObjMesh> meshes;
};

struct ObjLoadError {
    std::string message;
};

inline std::expected<ObjModel, ObjLoadError>
load_obj(const std::filesystem::path& path, bool triangulate = true) {
    ::tinyobj::ObjReaderConfig config;
    config.triangulate = triangulate;
    config.vertex_color = false;
    config.mtl_search_path = path.parent_path().string();

    ::tinyobj::ObjReader reader;
    if (!reader.ParseFromFile(path.string(), config)) {
        return std::unexpected(ObjLoadError{reader.Error()});
    }
    if (!reader.Warning().empty()) {
        std::println("tinyobj warning: {}", reader.Warning());
    }

    const auto& attrib = reader.GetAttrib();
    const auto& shapes = reader.GetShapes();
    const auto& materials = reader.GetMaterials();

    ObjModel model{};
    model.meshes.reserve(shapes.size());

    for (const auto& shape : shapes) {
        ObjMesh mesh{};
        mesh.indices.reserve(shape.mesh.indices.size());

        const bool has_normals = !attrib.normals.empty();
        const bool has_texcoords = !attrib.texcoords.empty();

        mesh.positions.reserve(shape.mesh.indices.size() * 3);
        if (has_normals)  mesh.normals.reserve(shape.mesh.indices.size() * 3);
        if (has_texcoords) mesh.texcoords.reserve(shape.mesh.indices.size() * 2);

        // Use the first material on this shape if present.
        if (!shape.mesh.material_ids.empty()) {
            const int mat_id = shape.mesh.material_ids.front();
            if (mat_id >= 0 && mat_id < static_cast<int>(materials.size())) {
                const auto& m = materials[static_cast<std::size_t>(mat_id)];
                mesh.diffuse = {m.diffuse[0], m.diffuse[1], m.diffuse[2]};
                mesh.diffuse_tex = m.diffuse_texname;
                mesh.specular_tex = m.specular_texname;
            }
        }

        for (const auto& idx : shape.mesh.indices) {
            const int vi = idx.vertex_index * 3;
            mesh.positions.push_back(attrib.vertices[vi + 0]);
            mesh.positions.push_back(attrib.vertices[vi + 1]);
            mesh.positions.push_back(attrib.vertices[vi + 2]);

            if (has_normals && idx.normal_index >= 0) {
                const int ni = idx.normal_index * 3;
                mesh.normals.push_back(attrib.normals[ni + 0]);
                mesh.normals.push_back(attrib.normals[ni + 1]);
                mesh.normals.push_back(attrib.normals[ni + 2]);
            }
            if (has_texcoords && idx.texcoord_index >= 0) {
                const int ti = idx.texcoord_index * 2;
                mesh.texcoords.push_back(attrib.texcoords[ti + 0]);
                mesh.texcoords.push_back(attrib.texcoords[ti + 1]);
            }

            mesh.indices.push_back(static_cast<unsigned int>(mesh.indices.size()));
        }

        model.meshes.push_back(std::move(mesh));
    }

    return model;
}

} // namespace render::tinyobj
