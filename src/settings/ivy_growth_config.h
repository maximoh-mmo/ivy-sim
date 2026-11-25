/*
 * Copyright (c) 2026 Max Heinze. All rights reserved.
 *
 * Licensed for non-commercial use with attribution only.
 * Commercial use is strictly prohibited.
 */

#pragma once
#include <cstdint>
/**
 * @file ivy_growth_config.h
 * @brief Configuration parameters for procedural ivy growth simulation.
 */

namespace ivygenerator {

/**
 * @struct IvyGrowthConfig
 * @brief Parameters steering ivy vine growth, stepping, and tropism weights.
 */
struct IvyGrowthConfig {
	float initial_thickness = 0.1f; /**< Initial vine thickness at root nodes. */
	float step_length = 0.1f; /**< Distance advanced along the surface per growth step. */
	float momentum_weight = 1.0f; /**< Weight given to continuing in the previous growth direction. */
	float upward_weight = 0.5f; /**< Weight given to upward phototropism/gravitropism (+Y axis). */
	float random_weight = 0.0f; /**< Weight given to random lateral wandering/deviation. */
	float attraction_weight = 0.5f; /**< Weight given to ascending the attraction scalar gradient. */
	float contour_weight = 0.5f; /**< Weight given to following attraction contour/isolines. */
	float gradient_epsilon = 0.01f; /**< Finite difference step in UV space for numerical gradient evaluation (> 0). */
	float branch_probability = 0.0f; /**< Per-step probability [0, 1] of spawning a side branch. */
	float branch_angle = 0.5235988f; /**< Branch deflection angle in radians (~30 degrees). */
	int32_t min_branch_interval = 0; /**< Minimum unbranched steps required before a tip may branch (>= 0). */
	int32_t max_points = 10000; /**< Maximum total points allowed in the growth graph. */
	float thickness_growth_per_step = 0.005f; /**< Base radial thickening per step for active vascular segments. */
	float max_thickness = 0.5f; /**< Maximum radial thickness a woody stem can reach. */
	float wither_rate_per_step = 0.002f; /**< Radial shrinkage per step for terminated branch segments. */
	float min_withered_thickness = 0.02f; /**< Minimum radial thickness preserved for visible dried twigs (> 0). */
	float gradient_saturation = 0.1f; /**< Step attraction-gain where steering reaches 50% strength (>= 0). */
	float crowding_radius = 0.0f; /**< Local spatial radius for self-avoidance and crowding checks (0 = disabled). */
	float avoidance_weight = 0.0f; /**< Steering weight repelling tips from nearby unrelated vines (>= 0). */
	int32_t max_crowding_neighbors = 0; /**< Neighbor count within crowding_radius that terminates a tip (0 = disabled). */
	float target_weight = 0.0f; /**< Steering weight pulling active tips toward unsatisfied target waypoints (>= 0). */
	float target_influence_radius = 0.0f; /**< Radius within which tips feel target attraction (0 = global pull). */
	float target_acceptance_radius = 0.2f; /**< Distance within which a target waypoint is marked satisfied (> 0). */
	int32_t max_top_edge_steps = 0; /**< Max horizontal steps a tip may trail along the top wall edge (0 = stop immediately). */
};

} // namespace ivygenerator
