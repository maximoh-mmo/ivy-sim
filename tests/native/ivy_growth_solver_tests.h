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
	bool test_step_attraction_gradient_steering();
	bool test_step_contour_adherence_follows_tangent();
	bool test_step_branching_waits_until_next_iteration();
	bool test_step_branching_determinism();
	bool test_step_respects_max_points();
	bool test_step_thickness_growth_and_withering();
	bool test_step_pipe_model_and_thickness_clamps();
	bool test_step_gradient_damping_scales_with_slope();
	bool test_step_min_branch_interval_enforced();
	bool test_step_avoidance_steering();
	bool test_step_crowding_termination();
	bool test_reject_invalid_target_points();
	bool test_step_target_steering_and_acceptance_continues_growth();
	bool test_step_target_influence_radius_scoping();
	bool test_connect_unreached_targets_guarantees_reachability();
	bool test_connect_unreached_targets_fails_when_max_points_exhausted();

	int m_passed = 0;
	int m_failed = 0;
};

} // namespace ivygenerator
