#ifndef GDPHYSICS_H
#define GDPHYSICS_H

#include <godot_cpp/classes/label.hpp>
#include <godot_cpp/classes/node.hpp>

namespace godot {

class GDPhysics : public Node
{
    GDCLASS(GDPhysics, Node)

private:
    double time_passed;
    double amplitude;
    NodePath label_path;
    NodePath wheel1;
    NodePath wheel2;
    NodePath wheel3;
    NodePath wheel4;
    NodePath body;
    NodePath track;

    static const int NOTIFICATION_EXTENSION_RELOADED = 2;

    void initialize();

protected:
    static void _bind_methods();

public:
    GDPhysics();
    ~GDPhysics();

    void _notification(int p_what);
    void _process(double delta) override;

    void set_amplitude(const double p_amplitude);
    double get_amplitude() const;

    void set_label(const NodePath& p_label);
    NodePath get_label() const;

    void set_wheel1(const NodePath& p_wheel1);
    NodePath get_wheel1() const;

    void set_wheel2(const NodePath& p_wheel2);
    NodePath get_wheel2() const;

    void set_wheel3(const NodePath& p_wheel3);
    NodePath get_wheel3() const;

    void set_wheel4(const NodePath& p_wheel4);
    NodePath get_wheel4() const;

    void set_body(const NodePath& p_body);
    NodePath get_body() const;

    void set_track(const NodePath& p_track);
    NodePath get_track() const;
};

}  // namespace godot

#endif
