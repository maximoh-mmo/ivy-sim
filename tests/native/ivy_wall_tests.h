/*
 * Ivy Sim - Procedural ivy generator for Godot 4
 * Copyright (C) 2026 Max Heinze
 *
 * SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
 * Dual-licensed under GNU GPLv3 (see LICENSE) or a commercial license (see COMMERCIAL_LICENSE.md).
 * Distributed WITHOUT ANY WARRANTY; see LICENSE for details.
 */

#pragma once

/**
 * @file ivy_wall_tests.h
 * @brief Unit test suite for IvyWall surface representation.
 */

/**
 * @class IvyWallTests
 * @brief Test suite verifying transformations, normal calculation, and surface testing of IvyWall.
 */
namespace ivygenerator {

class IvyWallTests {
public:
	/**
	 * @brief Executes all IvyWall unit tests and prints results to stdout.
	 * @return bool True if all tests passed.
	 */
	bool run_all();

private:
	bool test_local_to_world();
	bool test_world_to_local();
	bool test_round_trip();
	bool test_rotated_wall();
	bool test_get_normal();
	bool test_contains_local_point();
	bool test_local_to_uv();
	bool test_uv_scale_offset();
	bool test_is_on_surface();
	bool test_is_valid();

	/**
	 * @brief Assertion helper tracking passed and failed test count.
	 * @param condition Condition expression to evaluate.
	 * @param message Failure explanation printed when condition is false.
	 * @return bool True if condition was true.
	 */
	bool expect(bool condition, const char *message);

	int m_passed = 0; /**< Count of passing assertions. */
	int m_failed = 0; /**< Count of failing assertions. */
};

} // namespace ivygenerator
