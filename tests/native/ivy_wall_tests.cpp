/*
 * Copyright (c) 2026 Max Heinze. All rights reserved.
 *
 * Licensed for non-commercial use with attribution only.
 * Commercial use is strictly prohibited.
 */

/**
 * @file ivy_wall_tests.cpp
 * @brief Implementation of IvyWall planar test cases and assertions.
 */

#include "ivy_wall_tests.h"
#include "environment/ivy_wall.h"

#include <cmath>
#include <iostream>

namespace ivygenerator {
using namespace godot;

bool IvyWallTests::expect(bool condition, const char *message) {
	if (!condition) {
		std::cout << "    FAILED: " << message << '\n';
	}

	return condition;
}

static bool vector3_equal(const Vector3 &left, const Vector3 &right) {
	return std::abs(left.x - right.x) < 0.0001f &&
			std::abs(left.y - right.y) < 0.0001f &&
			std::abs(left.z - right.z) < 0.0001f;
}

static bool vector2_equal(const Vector2 &left, const Vector2 &right) {
	return std::abs(left.x - right.x) < 0.0001f &&
			std::abs(left.y - right.y) < 0.0001f;
}

bool IvyWallTests::test_local_to_world() {
	IvyWall wall;

	wall.origin = Vector3(10, 0, 5);
	wall.axis_u = Vector3(1, 0, 0);
	wall.axis_v = Vector3(0, 1, 0);

	const Vector3 result =
			wall.local_to_world(Vector2(3, 2));

	return expect(
			vector3_equal(result, Vector3(13, 2, 5)),
			"Local to world conversion incorrect");
}

bool IvyWallTests::test_world_to_local() {
	IvyWall wall;

	wall.origin = Vector3(10, 0, 5);
	wall.axis_u = Vector3(1, 0, 0);
	wall.axis_v = Vector3(0, 1, 0);

	const Vector2 result =
			wall.world_to_local(Vector3(13, 2, 5));

	return expect(
			vector2_equal(result, Vector2(3, 2)),
			"World to local conversion incorrect");
}

bool IvyWallTests::test_round_trip() {
	IvyWall wall;

	wall.origin = Vector3(4, 3, 2);
	wall.axis_u = Vector3(1, 0, 0);
	wall.axis_v = Vector3(0, 1, 0);

	const Vector2 original(5, 7);

	const Vector3 world =
			wall.local_to_world(original);

	const Vector2 result =
			wall.world_to_local(world);

	return expect(
			vector2_equal(result, original),
			"Coordinate round trip failed");
}

bool IvyWallTests::test_rotated_wall() {
	IvyWall wall;

	wall.origin = Vector3(0, 0, 0);
	wall.axis_u = Vector3(0, 0, 1);
	wall.axis_v = Vector3(0, 1, 0);

	const Vector2 local_position(3, 4);

	const Vector3 world =
			wall.local_to_world(local_position);

	const Vector2 result =
			wall.world_to_local(world);

	return expect(
			vector2_equal(result, local_position),
			"Rotated wall conversion failed");
}

bool IvyWallTests::test_get_normal() {
	IvyWall wall;

	wall.axis_u = Vector3(1, 0, 0);
	wall.axis_v = Vector3(0, 1, 0);

	const Vector3 result = wall.get_normal();

	return expect(
			vector3_equal(result, Vector3(0, 0, 1)),
			"Wall normal incorrect");
}

bool IvyWallTests::test_local_to_uv() {
	IvyWall wall;

	wall.width = 10.0f;
	wall.height = 5.0f;

	const Vector2 result = wall.local_to_uv(Vector2(5, 2.5f));

	return expect(
			vector2_equal(result, Vector2(0.5f, 0.5f)),
			"Basic UV conversion incorrect");
}

bool IvyWallTests::test_uv_scale_offset() {
	IvyWall wall;

	wall.width = 10.0f;
	wall.height = 5.0f;
	wall.uv_scale = Vector2(2.0f, 3.0f);
	wall.uv_offset = Vector2(0.1f, 0.2f);

	const Vector2 result = wall.local_to_uv(Vector2(5, 2.5f));

	return expect(
			vector2_equal(result, Vector2(1.1f, 1.7f)),
			"UV scale and offset incorrect");
}

bool IvyWallTests::test_contains_local_point() {
	IvyWall wall;

	wall.width = 10.0f;
	wall.height = 5.0f;

	return expect(wall.contains_local_point(Vector2(0, 0)),
				   "Wall should contain bottom-left corner") &
			expect(wall.contains_local_point(Vector2(10, 5)),
					"Wall should contain top-right corner") &
			expect(wall.contains_local_point(Vector2(5, 2)),
					"Wall should contain an interior point") &
			expect(!wall.contains_local_point(Vector2(11, 2)),
					"Wall should reject points beyond its width") &
			expect(!wall.contains_local_point(Vector2(5, 6)),
					"Wall should reject points beyond its height");
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

	const auto run = [this](const char *name, bool (IvyWallTests::*test)()) {
		std::cout << "Running: " << name << '\n';

		const bool result = (this->*test)();

		if (result) {
			++m_passed;
			std::cout << "    PASS\n";
		} else {
			++m_failed;
			std::cout << "    FAIL\n";
		}
	};

	run("Local to world", &IvyWallTests::test_local_to_world);
	run("World to local", &IvyWallTests::test_world_to_local);
	run("Round trip", &IvyWallTests::test_round_trip);
	run("Rotated wall", &IvyWallTests::test_rotated_wall);
	run("Get normal", &IvyWallTests::test_get_normal);
	run("Contains local point", &IvyWallTests::test_contains_local_point);
	run("Local to UV", &IvyWallTests::test_local_to_uv);
	run("UV scale and offset", &IvyWallTests::test_uv_scale_offset);
	run("Is on surface", &IvyWallTests::test_is_on_surface);
	run("Is valid", &IvyWallTests::test_is_valid);

	std::cout << "\nPassed: " << m_passed
			  << "\nFailed: " << m_failed << '\n';

	return m_failed == 0;
}

bool IvyWallTests::test_is_valid() {
	IvyWall wall;
	if (!expect(!wall.is_valid(), "Default wall with zero dimensions should be invalid")) {
		return false;
	}
	wall.width = 10.0f;
	wall.height = 5.0f;
	if (!expect(wall.is_valid(), "Configured orthogonal unit-axis wall should be valid")) {
		return false;
	}
	IvyWall negative_width = wall;
	negative_width.width = -1.0f;
	if (!expect(!negative_width.is_valid(), "Negative width should be invalid")) {
		return false;
	}
	IvyWall unnormalized_axis = wall;
	unnormalized_axis.axis_u = Vector3(2.0f, 0.0f, 0.0f);
	if (!expect(!unnormalized_axis.is_valid(), "Non-unit axis_u should be invalid")) {
		return false;
	}
	IvyWall non_orthogonal = wall;
	non_orthogonal.axis_v = Vector3(1.0f, 0.0f, 0.0f);
	if (!expect(!non_orthogonal.is_valid(), "Parallel axes should be invalid")) {
		return false;
	}
	IvyWall zero_uv_scale = wall;
	zero_uv_scale.uv_scale = Vector2(0.0f, 1.0f);
	if (!expect(!zero_uv_scale.is_valid(), "Zero UV scale should be invalid")) {
		return false;
	}
	return true;
}

} // namespace ivygenerator
