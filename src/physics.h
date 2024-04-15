#pragma once
#include <memory>
#include <stdint.h>
#include <array>
#include <vector>

namespace physics {

struct Matrix4 final
{
    std::array<float, 4> x_axis;
    std::array<float, 4> y_axis;
    std::array<float, 4> z_axis;
    std::array<float, 4> w_axis;
};

struct State final
{
    std::array<Matrix4, 4> wheel_transforms;
    Matrix4 body_transform;
    float speed;
    float rpm;
    int gear;
    bool last_finished;
    bool finished;
    int64_t step;
    uint32_t collected_checkpoints;
};

struct Vector3 final
{
    float x;
    float y;
    float z;
};

struct Vector4 final
{
    float x;
    float y;
    float z;
    float w;
};

enum class MeshKind : uint8_t
{
    Start = 0,
    Road = 1,
    Finish = 2,
    Checkpoint = 3,
};

struct Mesh final
{
    std::vector<std::array<float, 3>> vertices;
    std::vector<uint32_t> indices;
    Vector3 position;
    Vector4 rotation;
    Vector3 scale;
    MeshKind kind;
};

struct Track final
{
    std::vector<Mesh> meshes;
    Vector3 scale;
};

struct Input final
{
    bool up;
    bool down;
    bool left;
    bool right;
    bool brake;
    bool restart;
    bool respawn;
};

struct State;
struct Track;
struct Configuration;
struct Input;

class Physics
{
public:
    Physics(const Track& track, const Configuration& configuration);
    State simulate(const Input& input);
    std::string save_state() const;
    void load_state(const std::string& bytes);
    size_t checkpoint_count() const;
    size_t collected_checkpoint_count() const;

private:
    class impl;
    std::shared_ptr<impl> impl;
};

std::unique_ptr<Physics> new_physics(const Track& track, const Configuration& configuration);

}  // namespace physics
