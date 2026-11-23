/*
 * Copyright (c) 2026 Max Heinze. All rights reserved.
 *
 * Licensed for non-commercial use with attribution only.
 * Commercial use is strictly prohibited.
 */

#pragma once

/**
 * @file ivy_growth_solver_tests.h
 * @brief Unit test suite for IvyGrowthSolver simulation steps and boundary conditions.
 */

namespace ivygenerator {

/**
 * @class IvyGrowthSolverTests
 * @brief Test suite verifying solver initialization, step advancement, tropism, and boundary termination.
 */
class IvyGrowthSolverTests {
public:
	/**
	 * @brief Executes all IvyGrowthSolver unit tests and prints results.
	 * @return bool True if all tests passed.
	 */
	bool run_all();

private:
	bool run_test(const char *name, bool (IvyGrowthSolverTests::*test)());
	bool expect(bool condition, const char *message);

	bool test_initialize_single_root();
	bool test_initialize_multiple_roots();
	bool test_reject_invalid_wall();
	bool test_reject_invalid_sampler();
	bool test_reject_invalid_config();
	bool test_reject_invalid_start_points();
	bool test_failed_reinitialize_resets_state();
	bool test_step_uninitialized_returns_error();
	bool test_step_advances_single_root_upward();
	bool test_step_terminates_at_wall_boundary();
	bool test_step_multiple_roots_sequential();
	bool test_step_random_wander_determinism();
	bool test_step_momentum_and_upward_tropism();

	int m_passed = 0;
	int m_failed = 0;
};

} // namespace ivygenerator
