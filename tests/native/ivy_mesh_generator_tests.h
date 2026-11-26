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
 * @file ivy_mesh_generator_tests.h
 * @brief Unit test suite for native IvyMeshGenerator 3D stem tube generation.
 */

namespace ivygenerator {

/**
 * @class IvyMeshGeneratorTests
 * @brief Test suite verifying stem mesh validation, geometry, UVs, colors, winding, and branching.
 */
class IvyMeshGeneratorTests {
public:
	/**
	 * @brief Executes all IvyMeshGenerator unit tests and prints results.
	 * @return bool True if all tests passed.
	 */
	bool run_all();

private:
	bool run_test(const char *name, bool (IvyMeshGeneratorTests::*test)());
	bool expect(bool condition, const char *message);

	bool test_reject_invalid_inputs_and_clears_output();
	bool test_isolated_root_produces_empty_mesh();
	bool test_single_segment_geometry_uvs_and_winding();
	bool test_wall_adhesion_offset();
	bool test_vertex_color_channels();
	bool test_branching_y_junction_and_branch_order();
	bool test_end_to_end_solver_graph_to_mesh();
	bool test_leaf_reject_invalid_config_and_zero_density();
	bool test_leaf_orientation_handedness_and_scale();
	bool test_leaf_thickness_filtering();
	bool test_leaf_determinism_across_seeds();
	bool test_leaf_end_to_end_with_solver();

	int m_passed = 0;
	int m_failed = 0;
};

} // namespace ivygenerator