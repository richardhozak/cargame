#pragma once
#include <memory>
#include <stdint.h>
#include "rust/cxx.h"

namespace physics {

struct State;
struct Track;
struct Configuration;
struct Input;

class Physics
{
public:
    Physics(const Track& track, const Configuration& configuration);
    State simulate(const Input& input);
    rust::Vec<uint8_t> save_state() const;
    void load_state(rust::Slice<const uint8_t> bytes);

private:
    class impl;
    std::shared_ptr<impl> impl;
};

std::unique_ptr<Physics> new_physics(const Track& track, const Configuration& configuration);

}  // namespace physics
