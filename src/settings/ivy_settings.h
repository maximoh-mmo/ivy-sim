/*
 * Copyright (c) 2026 Max Heinze. All rights reserved.
 *
 * Licensed for non-commercial use with attribution only.
 * Commercial use is strictly prohibited.
 */

#pragma once

/**
 * @file ivy_settings.h
 * @brief Simulation configuration parameters and growth constraints.
 */

#include "ivy_growth_config.h"

#include <godot_cpp/classes/resource.hpp>

#include <cstdint>

namespace ivygenerator {

class IvySettings : public godot::Resource {
	GDCLASS(IvySettings, godot::Resource)

protected:
	static void _bind_methods();

public:
	// Simulation controls
	void set_seed(int32_t seed);
	int32_t get_seed() const;

	void set_start_count(int32_t start_count);
	int32_t get_start_count() const;

	void set_max_steps(int32_t max_steps);
	int32_t get_max_steps() const;

	// Growth geometry
	void set_initial_thickness(float initial_thickness);
	float get_initial_thickness() const;

	void set_step_length(float step_length);
	float get_step_length() const;

	void set_max_points(int32_t max_points);
	int32_t get_max_points() const;

	// Steering weights
	void set_momentum_weight(float momentum_weight);
	float get_momentum_weight() const;

	void set_upward_weight(float upward_weight);
	float get_upward_weight() const;

	void set_random_weight(float random_weight);
	float get_random_weight() const;

	void set_attraction_weight(float attraction_weight);
	float get_attraction_weight() const;

	void set_contour_weight(float contour_weight);
	float get_contour_weight() const;

	void set_gradient_epsilon(float gradient_epsilon);
	float get_gradient_epsilon() const;

	void set_gradient_saturation(float gradient_saturation);
	float get_gradient_saturation() const;

	// Branching
	void set_branch_probability(float branch_probability);
	float get_branch_probability() const;

	void set_branch_angle_degrees(float branch_angle_degrees);
	float get_branch_angle_degrees() const;

	void set_min_branch_interval(int32_t min_branch_interval);
	int32_t get_min_branch_interval() const;

	// Vascular thickening & withering
	void set_thickness_growth_per_step(float thickness_growth_per_step);
	float get_thickness_growth_per_step() const;

	void set_max_thickness(float max_thickness);
	float get_max_thickness() const;

	void set_wither_rate_per_step(float wither_rate_per_step);
	float get_wither_rate_per_step() const;

	void set_min_withered_thickness(float min_withered_thickness);
	float get_min_withered_thickness() const;

	void set_crowding_radius(float crowding_radius);
	float get_crowding_radius() const;

	void set_avoidance_weight(float avoidance_weight);
	float get_avoidance_weight() const;

	void set_max_crowding_neighbors(int32_t max_crowding_neighbours);
	int32_t get_max_crowding_neighbors() const;

	// Native conversion for IvyGrowthSolver
	IvyGrowthConfig to_growth_config() const;

private:
	int32_t m_seed = 42;
	int32_t m_start_count = 1;
	int32_t m_max_steps = 200;

	float m_initial_thickness = 0.1f;
	float m_step_length = 0.1f;
	int32_t m_max_points = 10000;

	float m_momentum_weight = 1.0f;
	float m_upward_weight = 0.5f;
	float m_random_weight = 0.0f;
	float m_attraction_weight = 0.5f;
	float m_contour_weight = 0.5f;
	float m_gradient_epsilon = 0.01f;
	float m_gradient_saturation = 0.1f;

	float m_branch_probability = 0.0f;
	float m_branch_angle_degrees = 30.0f;
	int32_t m_min_branch_interval = 0;

	float m_thickness_growth_per_step = 0.005f;
	float m_max_thickness = 0.5f;
	float m_wither_rate_per_step = 0.002f;
	float m_min_withered_thickness = 0.02f;

	float m_crowding_radius = 0.0f;
	float m_avoidance_weight = 0.0f;
	int32_t m_max_crowding_neighbors = 0;
};

} // namespace ivygenerator