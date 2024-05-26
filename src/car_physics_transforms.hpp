#pragma once

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/variant/packed_byte_array.hpp>
#include <godot_cpp/variant/transform3d.hpp>

namespace godot {

class CarPhysicsTransforms : public RefCounted
{
    GDCLASS(CarPhysicsTransforms, RefCounted)
public:
    static void _bind_methods();
    CarPhysicsTransforms();

    void set_body(const Transform3D& p_body);
    Transform3D get_body() const;

    void set_wheel1(const Transform3D& p_wheel1);
    Transform3D get_wheel1() const;

    void set_wheel2(const Transform3D& p_wheel2);
    Transform3D get_wheel2() const;

    void set_wheel3(const Transform3D& p_wheel3);
    Transform3D get_wheel3() const;

    void set_wheel4(const Transform3D& p_wheel4);
    Transform3D get_wheel4() const;

private:
    Transform3D body;
    Transform3D wheel1;
    Transform3D wheel2;
    Transform3D wheel3;
    Transform3D wheel4;
};

}  // namespace godot
