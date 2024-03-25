#ifndef GDPHYSICS_H
#define GDPHYSICS_H

#include <godot_cpp/classes/label.hpp>
#include <godot_cpp/classes/sprite2d.hpp>

namespace godot {

class GDPhysics : public Sprite2D
{
    GDCLASS(GDPhysics, Sprite2D)

private:
    double time_passed;
    double amplitude;
    NodePath label_path;

protected:
    static void _bind_methods();

public:
    GDPhysics();
    ~GDPhysics();

    void _ready() override;
    void _process(double delta) override;

    void set_amplitude(const double p_amplitude);
    double get_amplitude() const;

    void set_label(const NodePath& p_label);
    NodePath get_label() const;
};

}  // namespace godot

#endif
