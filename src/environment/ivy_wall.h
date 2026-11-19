#pragma once

#include <godot_cpp/variant/vector2.hpp>
#include <godot_cpp/variant/vector3.hpp>

namespace godot {

struct IvyWall {
    // World Space Properties
    Vector3 origin = Vector3();
    Vector3 axis_u = Vector3(1.0f, 0.0f, 0.0f);
    Vector3 axis_v = Vector3(0.0f, 1.0f, 0.0f);

    float width = 0.0f;
    float height = 0.0f;
    Vector2 uv_scale = Vector2(1.0f, 1.0f);
    Vector2 uv_offset = Vector2(0.0f, 0.0f);

    Vector2 world_to_local(const Vector3& world_position) const;
    Vector3 local_to_world(const Vector2& local_position) const;
    Vector2 local_to_uv(const Vector2& local_position) const;
    Vector3 get_normal() const;
    bool contains_local_point(const Vector2& local_position) const;
    bool is_on_surface(const Vector3& world_position, float tolerance = 0.01f) const;
};

} // namespace godot
