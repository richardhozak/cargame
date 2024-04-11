#include "car_physics_track_mesh.hpp"

#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

physics::MeshKind mesh_kind_from_color(const Color& color)
{
    if (color.is_equal_approx(Color{1.0, 0.0, 0.0, 1.0}))
    {
        return physics::MeshKind::Finish;
    }

    if (color.is_equal_approx(Color{0.0, 1.0, 0.0, 1.0}))
    {
        return physics::MeshKind::Start;
    }

    if (color.is_equal_approx(Color{0.0, 0.0, 1.0, 1.0}))
    {
        return physics::MeshKind::Checkpoint;
    }

    return physics::MeshKind::Road;
}

void CarPhysicsTrackMesh::_bind_methods()
{
    ClassDB::bind_method(D_METHOD("add_mesh", "position", "rotation", "scale", "vertices", "indices", "color"), &CarPhysicsTrackMesh::add_mesh);
}

bool CarPhysicsTrackMesh::add_mesh(
    Vector3 position,
    Quaternion rotation,
    Vector3 scale,
    PackedVector3Array vertices,
    PackedInt32Array indices,
    Color color)
{
    physics::MeshKind kind = mesh_kind_from_color(color);

    physics::Mesh physics_mesh;

    physics_mesh.kind = kind;
    physics_mesh.scale = physics::Vector3{scale.x, scale.y, scale.z};
    physics_mesh.position = physics::Vector3{position.x, position.y, position.z};
    physics_mesh.rotation = physics::Vector4{rotation.x, rotation.y, rotation.z, rotation.w};

    for (Vector3 vertex : vertices)
    {
        physics_mesh.vertices.push_back({vertex.x, vertex.y, vertex.z});
    }

    for (int32_t index : indices)
    {
        physics_mesh.indices.push_back(index);
    }

    meshes.push_back(std::move(physics_mesh));

    return kind == physics::MeshKind::Road;
}