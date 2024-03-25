#ifndef GDEXAMPLE_H
#define GDEXAMPLE_H

#include <godot_cpp/classes/label.hpp>
#include <godot_cpp/classes/sprite2d.hpp>

namespace godot {

class GDExample : public Sprite2D
{
    GDCLASS(GDExample, Sprite2D)

private:
    double time_passed;
    double amplitude;
    NodePath label_path;

protected:
    static void _bind_methods();

public:
    GDExample();
    ~GDExample();

    void _process(double delta) override;

    void set_amplitude(const double p_amplitude);
    double get_amplitude() const;

    void set_label(const NodePath& p_label);
    NodePath get_label() const;
};

}  // namespace godot

#endif
