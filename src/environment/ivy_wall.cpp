/*
 * Copyright (c) 2026 Max Heinze. All rights reserved.
 *
 * Licensed for non-commercial use with attribution only.
 * Commercial use is strictly prohibited.
 */

#include "ivy_wall.h"

#include <cmath>

using namespace godot;

Vector2 IvyWall::world_to_local(const Vector3& world_position) const {
    const Vector3 relative_position = world_position - origin;

    const float u = relative_position.dot(axis_u);
    const float v = relative_position.dot(axis_v);

    return Vector2(u, v);
}

Vector3 IvyWall::local_to_world(const Vector2& local_position) const {
    return origin + local_position.x * axis_u + local_position.y * axis_v;
}

Vector2 IvyWall::local_to_uv(const Vector2& local_position) const {
    if (width <= 0.0f || height <= 0.0f) {
        return uv_offset;
    }
    return Vector2(
        (local_position.x / width) * uv_scale.x + uv_offset.x,
        (local_position.y / height) * uv_scale.y + uv_offset.y
    );
}

Vector3 IvyWall::get_normal() const {
    const Vector3 computed = axis_u.cross(axis_v);
    const float length_sq = computed.length_squared();
    if (length_sq <= 0.000001f) {
        // Return a zero vector if the wall is degenerate (axis_u and axis_v are parallel or zero-length) 
        return Vector3();
    }
    return computed.normalized();
}

bool IvyWall::contains_local_point(const Vector2& local_position) const {
    return local_position.x >= 0.0f && local_position.x <= width &&
           local_position.y >= 0.0f && local_position.y <= height;
}

bool IvyWall::is_on_surface(const Vector3& world_position, float tolerance) const {
    const Vector3 diff = world_position - origin;
    const Vector3 n = get_normal();
    const float perp_distance = std::abs(diff.dot(n));
    if (perp_distance > tolerance) {
        return false;
    }

    const Vector2 local = world_to_local(world_position);
    return local.x >= -tolerance && local.x <= width + tolerance &&
           local.y >= -tolerance && local.y <= height + tolerance;
}
