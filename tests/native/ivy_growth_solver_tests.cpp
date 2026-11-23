/*
 * Copyright (c) 2026 Max Heinze. All rights reserved.
 *
 * Licensed for non-commercial use with attribution only.
 * Commercial use is strictly prohibited.
 */

/**
 * @file ivy_growth_solver_tests.cpp
 * @brief Implementation of IvyGrowthSolver unit tests.
 */

#include "ivy_growth_solver_tests.h"

#include "environment/ivy_surface_sampler.h"
#include "environment/ivy_wall.h"
#include "growth/ivy_growth_solver.h"
#include "random/ivy_random.h"
#include "settings/ivy_growth_config.h"

#include <iostream>
#include <vector>

namespace ivygenerator {
using namespace godot;

bool IvyGrowthSolverTests::run_test(
		const char *name,
		bool (IvyGrowthSolverTests::*test)()) {
	std::cout << "Running: " << name << '\n';

	const bool result = (this->*test)();

	if (result) {
		++m_passed;
		std::cout << "    PASS\n";
	} else {
		++m_failed;
		std::cout << "    FAIL\n";
	}

	return result;
}

bool IvyGrowthSolverTests::expect(bool condition, const char *message) {
	if (!condition) {
		std::cout << "    FAILED: " << message << '\n';
	}

	return condition;
}

bool IvyGrowthSolverTests::test_initialize_single_root() {
	IvyWall wall;
	wall.width = 10.0f;
	wall.height = 5.0f;

	IvySurfaceSampler sampler;
	sampler.set_pixel_data({ 1.0f }, 1, 1);

	IvyGrowthConfig config;
	config.initial_thickness = 0.25f;

	IvyRandom rng(42);
	const std::vector<Vector3> starts = { Vector3(2.0f, 0.0f, 0.0f) };

	IvyGrowthSolver solver;
	const bool initialized = solver.initialize(wall, sampler, config, rng, starts);
	if (!expect(initialized, "Valid single-root initialization should succeed")) {
		return false;
	}

	if (!expect(!solver.is_finished(), "Initialized solver should have active tips")) {
		return false;
	}

	const IvyGrowthGraph &graph = solver.get_graph();
	if (!expect(graph.get_point_count() == 1, "Graph should contain 1 root point")) {
		return false;
	}

	const IvyPoint *root = graph.get_point(0);
	return expect(root != nullptr, "Root point should be retrievable") &&
			expect(root->position == Vector3(2.0f, 0.0f, 0.0f), "Root position should match start point") &&
			expect(root->thickness == 0.25f, "Root thickness should match initial_thickness") &&
			expect(root->parent_index == -1, "Root parent_index should be -1");
}

bool IvyGrowthSolverTests::test_initialize_multiple_roots() {
	IvyWall wall;
	wall.width = 10.0f;
	wall.height = 5.0f;

	IvySurfaceSampler sampler;
	sampler.set_pixel_data({ 1.0f }, 1, 1);

	IvyGrowthConfig config;
	config.initial_thickness = 0.15f;

	IvyRandom rng(42);
	const std::vector<Vector3> starts = {
		Vector3(1.0f, 0.0f, 0.0f),
		Vector3(4.0f, 0.0f, 0.0f)
	};

	IvyGrowthSolver solver;
	if (!expect(solver.initialize(wall, sampler, config, rng, starts),
				"Valid multi-root initialization should succeed")) {
		return false;
	}

	const IvyGrowthGraph &graph = solver.get_graph();
	if (!expect(graph.get_point_count() == 2, "Graph should contain 2 root points")) {
		return false;
	}

	const IvyPoint *first = graph.get_point(0);
	const IvyPoint *second = graph.get_point(1);

	return expect(first != nullptr && second != nullptr, "Both root points should be retrievable") &&
			expect(first->position == starts[0] && second->position == starts[1], "Root positions should match start order") &&
			expect(first->parent_index == -1 && second->parent_index == -1, "Both points should be roots") &&
			expect(first->thickness == 0.15f && second->thickness == 0.15f, "Both roots should have initial_thickness");
}
bool IvyGrowthSolverTests::test_reject_invalid_wall() {
	IvyWall invalid_wall; // width and height default to 0.0f

	IvySurfaceSampler sampler;
	sampler.set_pixel_data({ 1.0f }, 1, 1);

	IvyGrowthConfig config;
	IvyRandom rng(42);
	const std::vector<Vector3> starts = { Vector3(0.0f, 0.0f, 0.0f) };

	IvyGrowthSolver solver;
	return expect(!solver.initialize(invalid_wall, sampler, config, rng, starts),
				   "Invalid wall should be rejected") &&
			expect(solver.get_graph().get_point_count() == 0, "Graph should remain empty") &&
			expect(solver.is_finished(), "Solver should report finished");
}

bool IvyGrowthSolverTests::test_reject_invalid_sampler() {
	IvyWall wall;
	wall.width = 10.0f;
	wall.height = 5.0f;

	IvySurfaceSampler invalid_sampler; // No pixel data loaded

	IvyGrowthConfig config;
	IvyRandom rng(42);
	const std::vector<Vector3> starts = { Vector3(2.0f, 0.0f, 0.0f) };

	IvyGrowthSolver solver;
	return expect(!solver.initialize(wall, invalid_sampler, config, rng, starts),
				   "Invalid surface sampler should be rejected") &&
			expect(solver.get_graph().get_point_count() == 0, "Graph should remain empty") &&
			expect(solver.is_finished(), "Solver should report finished");
}

bool IvyGrowthSolverTests::test_reject_invalid_config() {
	IvyWall wall;
	wall.width = 10.0f;
	wall.height = 5.0f;

	IvySurfaceSampler sampler;
	sampler.set_pixel_data({ 1.0f }, 1, 1);

	IvyRandom rng(42);
	const std::vector<Vector3> starts = { Vector3(2.0f, 0.0f, 0.0f) };

	IvyGrowthSolver solver;

	IvyGrowthConfig zero_thickness;
	zero_thickness.initial_thickness = 0.0f;
	if (!expect(!solver.initialize(wall, sampler, zero_thickness, rng, starts),
				"Zero initial_thickness should be rejected")) {
		return false;
	}

	IvyGrowthConfig negative_thickness;
	negative_thickness.initial_thickness = -0.5f;
	if (!expect(!solver.initialize(wall, sampler, negative_thickness, rng, starts),
				"Negative initial_thickness should be rejected")) {
		return false;
	}

	IvyGrowthConfig nan_thickness;
	nan_thickness.initial_thickness = std::numeric_limits<float>::quiet_NaN();
	if (!expect(!solver.initialize(wall, sampler, nan_thickness, rng, starts),
				"NaN initial_thickness should be rejected")) {
		return false;
	}
	IvyGrowthConfig zero_step;
	zero_step.step_length = 0.0f;
	if (!expect(!solver.initialize(wall, sampler, zero_step, rng, starts),
				"Zero step_length should be rejected")) {
		return false;
	}
	IvyGrowthConfig negative_weight;
	negative_weight.random_weight = -0.1f;
	if (!expect(!solver.initialize(wall, sampler, negative_weight, rng, starts),
				"Negative random_weight should be rejected")) {
		return false;
	}

	IvyGrowthConfig zero_directional_weights;
	zero_directional_weights.momentum_weight = 0.0f;
	zero_directional_weights.upward_weight = 0.0f;
	if (!expect(!solver.initialize(wall, sampler, zero_directional_weights, rng, starts),
				"Zero total directional weight should be rejected")) {
		return false;
	}

	IvyGrowthConfig negative_attraction;
	negative_attraction.attraction_weight = -0.1f;
	if (!expect(!solver.initialize(wall, sampler, negative_attraction, rng, starts),
				"Negative attraction_weight should be rejected")) {
		return false;
	}

	IvyGrowthConfig negative_contour;
	negative_contour.contour_weight = -0.1f;
	if (!expect(!solver.initialize(wall, sampler, negative_contour, rng, starts),
				"Negative contour_weight should be rejected")) {
		return false;
	}

	IvyGrowthConfig zero_epsilon;
	zero_epsilon.gradient_epsilon = 0.0f;
	if (!expect(!solver.initialize(wall, sampler, zero_epsilon, rng, starts),
				"Zero gradient_epsilon should be rejected")) {
		return false;
	}

	IvyGrowthConfig invalid_branch_prob;
	invalid_branch_prob.branch_probability = 1.1f;
	if (!expect(!solver.initialize(wall, sampler, invalid_branch_prob, rng, starts),
				"branch_probability > 1.0 should be rejected")) {
		return false;
	}

	IvyGrowthConfig invalid_branch_angle;
	invalid_branch_angle.branch_angle = 0.0f;
	if (!expect(!solver.initialize(wall, sampler, invalid_branch_angle, rng, starts),
				"Non-positive branch_angle should be rejected")) {
		return false;
	}

	IvyGrowthConfig zero_max_points;
	zero_max_points.max_points = 0;
	if (!expect(!solver.initialize(wall, sampler, zero_max_points, rng, starts),
				"Zero max_points should be rejected")) {
		return false;
	}

	IvyGrowthConfig too_few_max_points;
	too_few_max_points.max_points = 1;
	const std::vector<Vector3> two_starts = {
		Vector3(1.0f, 0.0f, 0.0f),
		Vector3(2.0f, 0.0f, 0.0f)
	};
	return expect(!solver.initialize(wall, sampler, too_few_max_points, rng, two_starts),
			"start_points exceeding max_points should be rejected");
}

bool IvyGrowthSolverTests::test_reject_invalid_start_points() {
	IvyWall wall;
	wall.width = 10.0f;
	wall.height = 5.0f;

	IvySurfaceSampler sampler;
	sampler.set_pixel_data({ 1.0f }, 1, 1);

	IvyGrowthConfig config;
	IvyRandom rng(42);
	IvyGrowthSolver solver;

	if (!expect(!solver.initialize(wall, sampler, config, rng, {}),
				"Empty start_points should be rejected")) {
		return false;
	}

	const std::vector<Vector3> off_plane = { Vector3(2.0f, 1.0f, 1.0f) };
	if (!expect(!solver.initialize(wall, sampler, config, rng, off_plane),
				"Start point off wall plane should be rejected")) {
		return false;
	}

	const std::vector<Vector3> second_out_of_bounds = {
		Vector3(2.0f, 0.0f, 0.0f),
		Vector3(25.0f, 0.0f, 0.0f)
	};
	return expect(!solver.initialize(wall, sampler, config, rng, second_out_of_bounds),
				   "Out-of-bounds second start point should be rejected") &&
			expect(solver.get_graph().get_point_count() == 0,
					"Failed multi-start check should not leave partial roots in graph");
}

bool IvyGrowthSolverTests::test_failed_reinitialize_resets_state() {
	IvyWall wall;
	wall.width = 10.0f;
	wall.height = 5.0f;

	IvySurfaceSampler sampler;
	sampler.set_pixel_data({ 1.0f }, 1, 1);

	IvyGrowthConfig config;
	IvyRandom rng(42);
	const std::vector<Vector3> valid_starts = { Vector3(2.0f, 0.0f, 0.0f) };

	IvyGrowthSolver solver;
	if (!expect(solver.initialize(wall, sampler, config, rng, valid_starts),
				"Initial valid setup should succeed")) {
		return false;
	}

	// Re-initialize with invalid empty starts.
	const bool reinitialized = solver.initialize(wall, sampler, config, rng, {});
	return expect(!reinitialized, "Re-initialization with empty starts should fail") &&
			expect(solver.get_graph().get_point_count() == 0, "Previous graph should be cleared on failure") &&
			expect(solver.is_finished(), "Active tips should be cleared on failure");
}
bool IvyGrowthSolverTests::test_step_uninitialized_returns_error() {
	IvyGrowthSolver solver;
	return expect(solver.step() == GrowthStepResult::ERROR,
			"Stepping an uninitialized solver should return ERROR");
}

bool IvyGrowthSolverTests::test_step_advances_single_root_upward() {
	IvyWall wall;
	wall.width = 10.0f;
	wall.height = 5.0f;

	IvySurfaceSampler sampler;
	sampler.set_pixel_data({ 1.0f }, 1, 1);

	IvyGrowthConfig config;
	config.initial_thickness = 0.2f;
	config.step_length = 0.5f;

	IvyRandom rng(42);
	const std::vector<Vector3> starts = { Vector3(2.0f, 0.0f, 0.0f) };

	IvyGrowthSolver solver;
	if (!expect(solver.initialize(wall, sampler, config, rng, starts),
				"Initialization should succeed")) {
		return false;
	}

	// Step 1: Root (index 0) advances straight up along axis_v to (2.0, 0.5, 0.0).
	if (!expect(solver.step() == GrowthStepResult::ADVANCED, "First step should return ADVANCED") ||
			!expect(!solver.is_finished(), "Solver should still have an active tip") ||
			!expect(solver.get_graph().get_point_count() == 2, "Graph should contain 2 points after first step")) {
		return false;
	}

	const IvyPoint *first_child = solver.get_graph().get_point(1);
	if (!expect(first_child != nullptr, "First child point should exist") ||
			!expect(first_child->parent_index == 0, "First child parent should be root 0") ||
			!expect(first_child->position.is_equal_approx(Vector3(2.0f, 0.5f, 0.0f)),
					"First child position should be (2.0, 0.5, 0.0)")) {
		return false;
	}

	// Step 2: Tip (index 1) derives direction from (tip - parent) and advances to (2.0, 1.0, 0.0).
	if (!expect(solver.step() == GrowthStepResult::ADVANCED, "Second step should return ADVANCED") ||
			!expect(solver.get_graph().get_point_count() == 3, "Graph should contain 3 points after second step")) {
		return false;
	}

	const IvyPoint *second_child = solver.get_graph().get_point(2);
	return expect(second_child != nullptr, "Second child point should exist") &&
			expect(second_child->parent_index == 1, "Second child parent should be point 1") &&
			expect(second_child->position.is_equal_approx(Vector3(2.0f, 1.0f, 0.0f)),
					"Second child position should be (2.0, 1.0, 0.0)");
}

bool IvyGrowthSolverTests::test_step_terminates_at_wall_boundary() {
	IvyWall wall;
	wall.width = 10.0f;
	wall.height = 1.0f;

	IvySurfaceSampler sampler;
	sampler.set_pixel_data({ 1.0f }, 1, 1);

	IvyGrowthConfig config;
	config.step_length = 0.6f;

	IvyRandom rng(42);
	const std::vector<Vector3> starts = { Vector3(2.0f, 0.0f, 0.0f) };

	IvyGrowthSolver solver;
	if (!expect(solver.initialize(wall, sampler, config, rng, starts),
				"Initialization should succeed")) {
		return false;
	}

	// Step 1: y = 0.0 -> y = 0.6 (inside height 1.0).
	if (!expect(solver.step() == GrowthStepResult::ADVANCED, "First step should advance") ||
			!expect(!solver.is_finished(), "Tip should still be active at y = 0.6")) {
		return false;
	}

	// Step 2: candidate y = 1.2 exceeds wall height 1.0 -> tip terminates.
	if (!expect(solver.step() == GrowthStepResult::ADVANCED, "Boundary iteration should return ADVANCED") ||
			!expect(solver.is_finished(), "Tip should terminate when stepping out of bounds") ||
			!expect(solver.get_graph().get_point_count() == 2, "Out-of-bounds point should not be added")) {
		return false;
	}

	// Step 3: no active tips remain.
	return expect(solver.step() == GrowthStepResult::FINISHED,
			"Subsequent step with no active tips should return FINISHED");
}

bool IvyGrowthSolverTests::test_step_multiple_roots_sequential() {
	IvyWall wall;
	wall.width = 10.0f;
	wall.height = 5.0f;

	IvySurfaceSampler sampler;
	sampler.set_pixel_data({ 1.0f }, 1, 1);

	IvyGrowthConfig config;
	config.step_length = 0.5f;

	IvyRandom rng(42);
	const std::vector<Vector3> starts = {
		Vector3(1.0f, 0.0f, 0.0f),
		Vector3(4.0f, 0.0f, 0.0f)
	};

	IvyGrowthSolver solver;
	if (!expect(solver.initialize(wall, sampler, config, rng, starts),
				"Multi-root initialization should succeed")) {
		return false;
	}

	if (!expect(solver.step() == GrowthStepResult::ADVANCED, "Multi-root step should return ADVANCED") ||
			!expect(solver.get_graph().get_point_count() == 4, "Both roots should add a child in one iteration")) {
		return false;
	}

	const IvyPoint *child_0 = solver.get_graph().get_point(2);
	const IvyPoint *child_1 = solver.get_graph().get_point(3);

	return expect(child_0 != nullptr && child_1 != nullptr, "Both child points should exist") &&
			expect(child_0->parent_index == 0 && child_1->parent_index == 1,
					"Children should preserve sequential active-tip order") &&
			expect(child_0->position.is_equal_approx(Vector3(1.0f, 0.5f, 0.0f)) &&
							child_1->position.is_equal_approx(Vector3(4.0f, 0.5f, 0.0f)),
					"Both children should advance upward from their respective roots");
}
bool IvyGrowthSolverTests::test_step_random_wander_determinism() {
	IvyWall wall;
	wall.width = 10.0f;
	wall.height = 5.0f;

	IvySurfaceSampler sampler;
	sampler.set_pixel_data({ 1.0f }, 1, 1);

	IvyGrowthConfig config;
	config.step_length = 0.5f;
	config.momentum_weight = 1.0f;
	config.upward_weight = 0.5f;
	config.random_weight = 0.8f;

	const std::vector<Vector3> starts = { Vector3(5.0f, 0.0f, 0.0f) };

	IvyRandom rng_a(42);
	IvyGrowthSolver solver_a;
	solver_a.initialize(wall, sampler, config, rng_a, starts);

	IvyRandom rng_b(42);
	IvyGrowthSolver solver_b;
	solver_b.initialize(wall, sampler, config, rng_b, starts);

	IvyRandom rng_c(99);
	IvyGrowthSolver solver_c;
	solver_c.initialize(wall, sampler, config, rng_c, starts);

	for (int i = 0; i < 3; ++i) {
		solver_a.step();
		solver_b.step();
		solver_c.step();
	}

	const IvyPoint *tip_a = solver_a.get_graph().get_point(3);
	const IvyPoint *tip_b = solver_b.get_graph().get_point(3);
	const IvyPoint *tip_c = solver_c.get_graph().get_point(3);

	if (!expect(tip_a != nullptr && tip_b != nullptr && tip_c != nullptr,
				"All three solvers should produce 3 steps")) {
		return false;
	}

	return expect(std::abs(tip_a->position.x - 5.0f) > 0.001f,
				   "Random wander should deflect vine laterally off x = 5.0") &&
			expect(tip_a->position.is_equal_approx(tip_b->position),
					"Identical seeds should produce identical wandered paths") &&
			expect(!tip_a->position.is_equal_approx(tip_c->position),
					"Different seeds should produce different wandered paths");
}

bool IvyGrowthSolverTests::test_step_momentum_and_upward_tropism() {
	IvyWall wall;
	wall.width = 10.0f;
	wall.height = 5.0f;

	IvySurfaceSampler sampler;
	sampler.set_pixel_data({ 1.0f }, 1, 1);

	const std::vector<Vector3> starts = { Vector3(5.0f, 0.0f, 0.0f) };

	// Solver 1: Pure momentum on step 2 (upward_weight = 0.0f).
	IvyGrowthConfig momentum_only;
	momentum_only.step_length = 0.5f;
	momentum_only.momentum_weight = 1.0f;
	momentum_only.upward_weight = 0.0f;
	momentum_only.random_weight = 1.0f;

	IvyRandom rng_1(42);
	IvyGrowthSolver solver_momentum;
	solver_momentum.initialize(wall, sampler, momentum_only, rng_1, starts);
	solver_momentum.step(); // Step 1: deflects at an angle
	momentum_only.random_weight = 0.0f; // Disable randomness for Step 2
	solver_momentum.step();

	const Vector3 p0_m = solver_momentum.get_graph().get_point(0)->position;
	const Vector3 p1_m = solver_momentum.get_graph().get_point(1)->position;
	const Vector3 p2_m = solver_momentum.get_graph().get_point(2)->position;
	const Vector3 dir1_m = (p1_m - p0_m).normalized();
	const Vector3 dir2_m = (p2_m - p1_m).normalized();

	if (!expect(dir1_m.is_equal_approx(dir2_m),
				"With zero upward_weight and zero random_weight, momentum should preserve exact heading")) {
		return false;
	}

	// Solver 2: Same tilted Step 1, then upward tropism enabled on Step 2.
	IvyGrowthConfig with_upward = momentum_only;
	with_upward.random_weight = 1.0f;

	IvyRandom rng_2(42);
	IvyGrowthSolver solver_upward;
	solver_upward.initialize(wall, sampler, with_upward, rng_2, starts);
	solver_upward.step(); // Identical tilted Step 1
	with_upward.random_weight = 0.0f;
	with_upward.upward_weight = 1.0f;
	solver_upward.step(); // Step 2 curves back toward vertical

	const Vector3 p1_u = solver_upward.get_graph().get_point(1)->position;
	const Vector3 p2_u = solver_upward.get_graph().get_point(2)->position;
	const Vector3 dir2_u = (p2_u - p1_u).normalized();

	return expect(dir2_u.y > dir1_m.y + 0.001f,
			"Upward tropism should bend tilted growth direction back toward +Y");
}

bool IvyGrowthSolverTests::test_step_attraction_gradient_steering() {
	IvyWall wall;
	wall.width = 10.0f;
	wall.height = 10.0f;

	IvyGrowthConfig config;
	config.step_length = 0.5f;
	config.momentum_weight = 1.0f;
	config.upward_weight = 0.0f;
	config.random_weight = 0.0f;
	config.attraction_weight = 1.0f;
	config.contour_weight = 0.0f;

	const std::vector<Vector3> starts = { Vector3(5.0f, 5.0f, 0.0f) };

	// Right column bright (1.0), left column dark (0.0) -> gradient points +X.
	IvySurfaceSampler right_bright_sampler;
	right_bright_sampler.set_pixel_data({ 0.0f, 1.0f,
												0.0f, 1.0f },
			2, 2);

	IvyRandom rng_right(42);
	IvyGrowthSolver solver_right;
	solver_right.initialize(wall, right_bright_sampler, config, rng_right, starts);
	solver_right.step();

	const IvyPoint *right_tip = solver_right.get_graph().get_point(1);
	if (!expect(right_tip != nullptr && right_tip->position.x > 5.0f,
				"Attraction gradient should steer tip toward brighter right side")) {
		return false;
	}

	// Left column bright (1.0), right column dark (0.0) -> gradient points -X.
	IvySurfaceSampler left_bright_sampler;
	left_bright_sampler.set_pixel_data({ 1.0f, 0.0f,
											   1.0f, 0.0f },
			2, 2);

	IvyRandom rng_left(42);
	IvyGrowthSolver solver_left;
	solver_left.initialize(wall, left_bright_sampler, config, rng_left, starts);
	solver_left.step();

	const IvyPoint *left_tip = solver_left.get_graph().get_point(1);
	return expect(left_tip != nullptr && left_tip->position.x < 5.0f,
			"Attraction gradient should steer tip toward brighter left side");
}

bool IvyGrowthSolverTests::test_step_contour_adherence_follows_tangent() {
	IvyWall wall;
	wall.width = 10.0f;
	wall.height = 10.0f;

	// Diagonal field: top-left is brightest (1.0), bottom-right is darkest (0.0).
	// At (0.5, 0.5), gradient points up-left (-1, +1), while the upward-aligned
	// iso-contour (connecting the two 0.5 pixels) points up-right (+1, +1).
	IvySurfaceSampler diagonal_sampler;
	diagonal_sampler.set_pixel_data({ 1.0f, 0.5f,
											0.5f, 0.0f },
			2, 2);
	const std::vector<Vector3> starts = { Vector3(5.0f, 5.0f, 0.0f) };

	// Pure attraction (steers left toward brighter bottom-left).
	IvyGrowthConfig attract_only;
	attract_only.step_length = 0.5f;
	attract_only.momentum_weight = 1.0f;
	attract_only.upward_weight = 0.5f;
	attract_only.attraction_weight = 1.0f;
	attract_only.contour_weight = 0.0f;

	IvyRandom rng_a(42);
	IvyGrowthSolver solver_attract;
	solver_attract.initialize(wall, diagonal_sampler, attract_only, rng_a, starts);
	solver_attract.step();

	// Pure contour adherence (steers right along the upward-right diagonal iso-contour).
	IvyGrowthConfig contour_only = attract_only;
	contour_only.attraction_weight = 0.0f;
	contour_only.contour_weight = 1.0f;

	IvyRandom rng_c(42);
	IvyGrowthSolver solver_contour;
	solver_contour.initialize(wall, diagonal_sampler, contour_only, rng_c, starts);
	solver_contour.step();

	const IvyPoint *tip_attract = solver_attract.get_graph().get_point(1);
	const IvyPoint *tip_contour = solver_contour.get_graph().get_point(1);

	return expect(tip_attract != nullptr && tip_contour != nullptr,
				   "Both attraction and contour steps should produce a point") &&
			expect(tip_attract->position.x < 5.0f,
					"Attraction-only step should steer toward brighter left side (x < 5.0)") &&
			expect(tip_contour->position.x > 5.0f,
					"Contour-only step should follow upward-right contour tangent (x > 5.0)");
}
bool IvyGrowthSolverTests::test_step_branching_waits_until_next_iteration() {
	IvyWall wall;
	wall.width = 10.0f;
	wall.height = 10.0f;

	IvySurfaceSampler sampler;
	sampler.set_pixel_data({ 1.0f }, 1, 1);

	IvyGrowthConfig config;
	config.step_length = 0.5f;
	config.momentum_weight = 1.0f;
	config.upward_weight = 0.0f;
	config.branch_probability = 1.0f;
	config.branch_angle = 0.5f;

	IvyRandom rng(42);
	const std::vector<Vector3> starts = { Vector3(5.0f, 0.0f, 0.0f) };

	IvyGrowthSolver solver;
	if (!expect(solver.initialize(wall, sampler, config, rng, starts),
				"Initialization should succeed")) {
		return false;
	}

	// Step 1: Root (0) produces main stem (1) and side branch (2), but (2) is not stepped yet.
	if (!expect(solver.step() == GrowthStepResult::ADVANCED, "Step 1 should advance") ||
			!expect(solver.get_graph().get_point_count() == 3,
					"Step 1 should add main point and branch point without stepping branch yet")) {
		return false;
	}

	const IvyPoint *main_1 = solver.get_graph().get_point(1);
	const IvyPoint *branch_1 = solver.get_graph().get_point(2);
	if (!expect(main_1 != nullptr && branch_1 != nullptr, "Main and branch points should exist") ||
			!expect(main_1->parent_index == 0 && branch_1->parent_index == 0,
					"Main and branch points should share root parent 0") ||
			!expect(std::abs(main_1->position.x - 5.0f) < 0.0001f,
					"Main stem should grow straight up") ||
			!expect(std::abs(branch_1->position.x - 5.0f) > 0.1f,
					"Side branch should deflect laterally off x = 5.0")) {
		return false;
	}

	// Step 2: Disable further branching; both main tip (1) and branch tip (2) advance once.
	config.branch_probability = 0.0f;
	if (!expect(solver.step() == GrowthStepResult::ADVANCED, "Step 2 should advance") ||
			!expect(solver.get_graph().get_point_count() == 5,
					"Step 2 should advance both main tip and new branch tip")) {
		return false;
	}

	const IvyPoint *main_2 = solver.get_graph().get_point(3);
	const IvyPoint *branch_2 = solver.get_graph().get_point(4);
	const float branch_dx_1 = branch_1->position.x - 5.0f;
	const float branch_dx_2 = branch_2->position.x - branch_1->position.x;

	return expect(main_2 != nullptr && branch_2 != nullptr, "Step 2 tips should exist") &&
			expect(main_2->parent_index == 1 && branch_2->parent_index == 2,
					"Step 2 should advance tips 1 and 2 in sequential order") &&
			expect((branch_dx_1 * branch_dx_2) > 0.0f,
					"Branch tip should preserve lateral heading on its first step");
}

bool IvyGrowthSolverTests::test_step_branching_determinism() {
	IvyWall wall;
	wall.width = 10.0f;
	wall.height = 10.0f;

	IvySurfaceSampler sampler;
	sampler.set_pixel_data({ 1.0f }, 1, 1);

	IvyGrowthConfig config;
	config.step_length = 0.4f;
	config.random_weight = 0.3f;
	config.branch_probability = 0.5f;
	config.branch_angle = 0.5f;

	const std::vector<Vector3> starts = { Vector3(5.0f, 0.0f, 0.0f) };

	IvyRandom rng_a(123);
	IvyGrowthSolver solver_a;
	solver_a.initialize(wall, sampler, config, rng_a, starts);

	IvyRandom rng_b(123);
	IvyGrowthSolver solver_b;
	solver_b.initialize(wall, sampler, config, rng_b, starts);

	for (int i = 0; i < 5; ++i) {
		solver_a.step();
		solver_b.step();
	}

	const IvyGrowthGraph &graph_a = solver_a.get_graph();
	const IvyGrowthGraph &graph_b = solver_b.get_graph();
	if (!expect(graph_a.get_point_count() == graph_b.get_point_count(),
				"Identical seeds should produce identical branched point counts")) {
		return false;
	}

	for (int32_t i = 0; i < graph_a.get_point_count(); ++i) {
		const IvyPoint *a = graph_a.get_point(i);
		const IvyPoint *b = graph_b.get_point(i);
		if (!expect(a->parent_index == b->parent_index &&
							a->position.is_equal_approx(b->position),
					"Identical seeds should produce identical branch topology and positions")) {
			return false;
		}
	}

	return true;
}

bool IvyGrowthSolverTests::test_step_respects_max_points() {
	IvyWall wall;
	wall.width = 10.0f;
	wall.height = 10.0f;

	IvySurfaceSampler sampler;
	sampler.set_pixel_data({ 1.0f }, 1, 1);

	IvyGrowthConfig config;
	config.step_length = 0.5f;
	config.branch_probability = 1.0f;
	config.max_points = 4;

	IvyRandom rng(42);
	const std::vector<Vector3> starts = { Vector3(5.0f, 0.0f, 0.0f) };

	IvyGrowthSolver solver;
	if (!expect(solver.initialize(wall, sampler, config, rng, starts),
				"Initialization should succeed")) {
		return false;
	}

	// Step 1: 1 -> 3 points (root 0, main 1, branch 2).
	if (!expect(solver.step() == GrowthStepResult::ADVANCED, "Step 1 should advance") ||
			!expect(solver.get_graph().get_point_count() == 3, "Graph should have 3 points after Step 1") ||
			!expect(!solver.is_finished(), "Solver should not be finished before reaching max_points")) {
		return false;
	}

	// Step 2: 3 -> 4 points (hits max_points and stops cleanly).
	if (!expect(solver.step() == GrowthStepResult::ADVANCED, "Step 2 should return ADVANCED") ||
			!expect(solver.get_graph().get_point_count() == 4, "Graph should stop at exactly max_points = 4") ||
			!expect(solver.is_finished(), "Solver should report finished once max_points is reached")) {
		return false;
	}

	// Step 3: already finished.
	return expect(solver.step() == GrowthStepResult::FINISHED,
				   "Subsequent step after max_points should return FINISHED") &&
			expect(solver.get_graph().get_point_count() == 4,
					"Graph size should remain capped at 4");
}

bool IvyGrowthSolverTests::run_all() {
	m_passed = 0;
	m_failed = 0;

	std::cout << "\n=== IvyGrowthSolver Unit Tests ===\n\n";

	run_test("Initialize single root",
			&IvyGrowthSolverTests::test_initialize_single_root);

	run_test("Initialize multiple roots",
			&IvyGrowthSolverTests::test_initialize_multiple_roots);

	run_test("Reject invalid wall",
			&IvyGrowthSolverTests::test_reject_invalid_wall);

	run_test("Reject invalid sampler",
			&IvyGrowthSolverTests::test_reject_invalid_sampler);

	run_test("Reject invalid config",
			&IvyGrowthSolverTests::test_reject_invalid_config);

	run_test("Reject invalid start points",
			&IvyGrowthSolverTests::test_reject_invalid_start_points);

	run_test("Failed reinitialize resets state",
			&IvyGrowthSolverTests::test_failed_reinitialize_resets_state);

	run_test("Step uninitialized returns error",
			&IvyGrowthSolverTests::test_step_uninitialized_returns_error);

	run_test("Step advances single root upward",
			&IvyGrowthSolverTests::test_step_advances_single_root_upward);

	run_test("Step terminates at wall boundary",
			&IvyGrowthSolverTests::test_step_terminates_at_wall_boundary);

	run_test("Step multiple roots sequential",
			&IvyGrowthSolverTests::test_step_multiple_roots_sequential);

	run_test("Step random wander determinism",
			&IvyGrowthSolverTests::test_step_random_wander_determinism);

	run_test("Step momentum and upward tropism",
			&IvyGrowthSolverTests::test_step_momentum_and_upward_tropism);

	run_test("Step attraction gradient steering",
			&IvyGrowthSolverTests::test_step_attraction_gradient_steering);

	run_test("Step contour adherence follows tangent",
			&IvyGrowthSolverTests::test_step_contour_adherence_follows_tangent);

	run_test("Step branching waits until next iteration",
			&IvyGrowthSolverTests::test_step_branching_waits_until_next_iteration);

	run_test("Step branching determinism",
			&IvyGrowthSolverTests::test_step_branching_determinism);

	run_test("Step respects max_points",
			&IvyGrowthSolverTests::test_step_respects_max_points);

	std::cout << "\n=== Test Summary ===\n";
	std::cout << "Passed: " << m_passed << '\n';
	std::cout << "Failed: " << m_failed << '\n';

	return m_failed == 0;
}

} // namespace ivygenerator