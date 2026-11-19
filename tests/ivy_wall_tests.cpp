#include "ivy_wall_tests.h"
#include "../src/environment/ivy_wall.h"

#include <iostream>
#include <cmath>

using namespace godot;

bool IvyWallTests::expect(bool condition, const char* message) {
    if (!condition) {
        std::cout << "    FAILED: " << message << '\n';
    }

    return condition;
}

static bool vector3_equal(const Vector3& left, const Vector3& right, float epsilon = 0.0001f) {
    return
        std::abs(left.x - right.x) < epsilon &&
        std::abs(left.y - right.y) < epsilon &&
        std::abs(left.z - right.z) < epsilon;
}

static bool vector2_equal(const Vector2& left, const Vector2& right, float epsilon = 0.0001f) {
    return
        std::abs(left.x - right.x) < epsilon &&
        std::abs(left.y - right.y) < epsilon;
}

bool IvyWallTests::test_local_to_world() {
    IvyWall wall;
    wall.origin = Vector3(10, 0, 5);
    wall.axis_u = Vector3(1, 0, 0);
    wall.axis_v = Vector3(0, 1, 0);

    const Vector3 result = wall.local_to_world(Vector2(3, 2));

    return expect(
        vector3_equal(result, Vector3(13, 2, 5)),
        "Local to world conversion incorrect"
    );
}

bool IvyWallTests::test_world_to_local() {
    IvyWall wall;
    wall.origin = Vector3(10, 0, 5);
    wall.axis_u = Vector3(1, 0, 0);
    wall.axis_v = Vector3(0, 1, 0);

    const Vector2 result = wall.world_to_local(Vector3(13, 2, 5));

    return expect(
        vector2_equal(result, Vector2(3, 2)),
        "World to local conversion incorrect"
    );
}

bool IvyWallTests::test_round_trip() {
    IvyWall wall;
    wall.origin = Vector3(4, 3, 2);
    wall.axis_u = Vector3(1, 0, 0);
    wall.axis_v = Vector3(0, 1, 0);

    const Vector2 original(5, 7);
    const Vector3 world = wall.local_to_world(original);
    const Vector2 result = wall.world_to_local(world);

    return expect(
        vector2_equal(result, original),
        "Coordinate round trip failed"
    );
}

bool IvyWallTests::test_rotated_wall() {
    IvyWall wall;
    wall.origin = Vector3(0, 0, 0);
    wall.axis_u = Vector3(0, 0, 1);
    wall.axis_v = Vector3(0, 1, 0);

    const Vector2 local_position(3, 4);
    const Vector3 world = wall.local_to_world(local_position);
    const Vector2 result = wall.world_to_local(world);

    return expect(
        vector2_equal(result, local_position),
        "Rotated wall conversion failed"
    );
}

bool IvyWallTests::test_get_normal() {
    IvyWall wall;
    wall.axis_u = Vector3(1, 0, 0);
    wall.axis_v = Vector3(0, 1, 0);

    const Vector3 n = wall.get_normal();
    return expect(
        vector3_equal(n, Vector3(0, 0, 1)),
        "Normal calculation incorrect (expected Z+ normal)"
    );
}

bool IvyWallTests::test_contains_local_point() {
    IvyWall wall;
    wall.width = 10.0f;
    wall.height = 5.0f;

    if (!expect(wall.contains_local_point(Vector2(5.0f, 2.5f)), "Center point should be inside")) {
        return false;
    }
    if (!expect(wall.contains_local_point(Vector2(0.0f, 0.0f)), "Origin point should be inside")) {
        return false;
    }
    if (!expect(wall.contains_local_point(Vector2(10.0f, 5.0f)), "Top-right point should be inside")) {
        return false;
    }
    if (!expect(!wall.contains_local_point(Vector2(-0.1f, 2.0f)), "Negative X should be outside")) {
        return false;
    }
    if (!expect(!wall.contains_local_point(Vector2(10.1f, 2.0f)), "X > width should be outside")) {
        return false;
    }
    if (!expect(!wall.contains_local_point(Vector2(5.0f, -0.1f)), "Negative Y should be outside")) {
        return false;
    }
    if (!expect(!wall.contains_local_point(Vector2(5.0f, 5.1f)), "Y > height should be outside")) {
        return false;
    }

    return true;
}

bool IvyWallTests::test_local_to_uv() {
    IvyWall wall;
    wall.width = 10.0f;
    wall.height = 4.0f;
    wall.uv_scale = Vector2(1.0f, 1.0f);
    wall.uv_offset = Vector2(0.0f, 0.0f);

    const Vector2 uv_center = wall.local_to_uv(Vector2(5.0f, 2.0f));
    if (!expect(vector2_equal(uv_center, Vector2(0.5f, 0.5f)), "Center UV should be (0.5, 0.5)")) {
        return false;
    }

    wall.uv_scale = Vector2(2.0f, 2.0f);
    wall.uv_offset = Vector2(0.1f, 0.1f);
    const Vector2 uv_scaled = wall.local_to_uv(Vector2(5.0f, 2.0f));
    return expect(
        vector2_equal(uv_scaled, Vector2(1.1f, 1.1f)),
        "Scaled/offset UV calculation failed"
    );
}

bool IvyWallTests::test_is_on_surface() {
    IvyWall wall;
    wall.origin = Vector3(10.0f, 0.0f, 5.0f);
    wall.axis_u = Vector3(1.0f, 0.0f, 0.0f);
    wall.axis_v = Vector3(0.0f, 1.0f, 0.0f);
    wall.width = 10.0f;
    wall.height = 5.0f;

    // Exact surface point (X=15, Y=2, Z=5)
    if (!expect(wall.is_on_surface(Vector3(15.0f, 2.0f, 5.0f), 0.01f), "Surface point should be detected")) {
        return false;
    }

    // Slightly off plane within tolerance
    if (!expect(wall.is_on_surface(Vector3(15.0f, 2.0f, 5.05f), 0.1f), "Point within normal tolerance should be on surface")) {
        return false;
    }

    // Off plane beyond tolerance
    if (!expect(!wall.is_on_surface(Vector3(15.0f, 2.0f, 5.2f), 0.1f), "Point beyond normal tolerance should not be on surface")) {
        return false;
    }

    // On plane but outside width bounds
    if (!expect(!wall.is_on_surface(Vector3(25.0f, 2.0f, 5.0f), 0.01f), "Point outside width bounds should not be on surface")) {
        return false;
    }

    return true;
}

bool IvyWallTests::run_all() {
    m_passed = 0;
    m_failed = 0;

    std::cout << "\n=== IvyWall Unit Tests ===\n\n";

    const auto run = [this](const char* name, bool result) {
        std::cout << "Running: " << name << '\n';

        if (result) {
            ++m_passed;
            std::cout << "    PASS\n";
        } else {
            ++m_failed;
            std::cout << "    FAIL\n";
        }
    };

    run("Local to world", test_local_to_world());
    run("World to local", test_world_to_local());
    run("Round trip", test_round_trip());
    run("Rotated wall", test_rotated_wall());
    run("Get normal", test_get_normal());
    run("Contains local point", test_contains_local_point());
    run("Local to UV", test_local_to_uv());
    run("Is on surface", test_is_on_surface());

    std::cout << "\nPassed: " << m_passed
              << "\nFailed: " << m_failed << '\n';

    return m_failed == 0;
}
