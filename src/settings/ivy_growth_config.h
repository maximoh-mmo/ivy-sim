/*
 * Copyright (c) 2026 Max Heinze. All rights reserved.
 *
 * Licensed for non-commercial use with attribution only.
 * Commercial use is strictly prohibited.
 */

#pragma once

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
	int32_t max_points = 10000; /**< Maximum total points allowed in the growth graph. */
};

} // namespace ivygenerator
