/*
 * Ivy Sim - Procedural ivy generator for Godot 4
 * Copyright (C) 2026 Max Heinze
 *
 * SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
 * Dual-licensed under GNU GPLv3 (see LICENSE) or a commercial license (see COMMERCIAL_LICENSE.md).
 * Distributed WITHOUT ANY WARRANTY; see LICENSE for details.
 */

/**
 * @file test_main.cpp
 * @brief Standalone test runner entry point for all ivy-sim C++ unit tests.
 */

#include "ivy_growth_graph_tests.h"
#include "ivy_growth_solver_tests.h"
#include "ivy_mesh_generator_tests.h"
#include "ivy_pixel_sampler_tests.h"
#include "ivy_random_tests.h"
#include "ivy_wall_tests.h"

using namespace ivygenerator;

/**
 * @brief Main test runner function executing IvyRandom, IvyWall, and IvyPixelSampler test suites.
 * @return int 0 if all test suites pass, 1 if any failure occurs.
 */
int main() {
	IvyRandomTests random_tests;
	IvyWallTests wall_tests;
	IvyPixelSamplerTests pixel_sampler_tests;
	IvyGrowthGraphTests graph_tests;
	IvyGrowthSolverTests solver_tests;
	IvyMeshGeneratorTests mesh_generator_tests;
	const bool random_passed = random_tests.run_all();
	const bool wall_passed = wall_tests.run_all();
	const bool pixel_sampler_passed = pixel_sampler_tests.run_all();
	const bool ivy_growth_graph_passed = graph_tests.run_all();
	const bool solver_passed = solver_tests.run_all();
	const bool mesh_generator_passed = mesh_generator_tests.run_all();

	return random_passed && wall_passed && pixel_sampler_passed && ivy_growth_graph_passed && solver_passed && mesh_generator_passed ? 0 : 1;
}
