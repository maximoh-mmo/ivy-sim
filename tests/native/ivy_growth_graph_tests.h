/*
 * Ivy Sim - Procedural ivy generator for Godot 4
 * Copyright (C) 2026 Max Heinze
 *
 * SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
 * Dual-licensed under GNU GPLv3 (see LICENSE) or a commercial license (see COMMERCIAL_LICENSE.md).
 * Distributed WITHOUT ANY WARRANTY; see LICENSE for details.
 */

#pragma once

#include "growth/ivy_growth_graph.h"

/**
 * @file ivy_growth_graph_tests.h
 * @brief Unit test suite for IvyGrowthGraph point insertion and hierarchy validation.
 */

namespace ivygenerator {

/**
 * @class IvyGrowthGraphTests
 * @brief Unit test runner verifying IvyGrowthGraph node management and connectivity rules.
 */
class IvyGrowthGraphTests {
private:
	int tests_passed = 0; /**< Counter of passed test assertions. */
	int tests_failed = 0; /**< Counter of failed test assertions. */

	void expect(const char *test_name, bool condition);

	void test_add_root();
	void test_add_child();
	void test_branching();
	void test_invalid_negative_parent();
	void test_invalid_parent_index();
	void test_get_point();
	void test_clear();
	void test_set_point_thickness();

public:
	/**
	 * @brief Executes all IvyGrowthGraph unit tests and prints the summary.
	 * @return bool True if all tests passed, false if any test failed.
	 */
	bool run_all();
};

} // namespace ivygenerator
