/*
 * Copyright (c) 2026 Max Heinze. All rights reserved.
 *
 * Licensed for non-commercial use with attribution only.
 * Commercial use is strictly prohibited.
 */

/**
 * @file ivy_settings.cpp
 * @brief Implementation of ivy configuration parameters.
 */

#include "ivy_settings.h"
#include <godot_cpp/core/class_db.hpp>

namespace ivygenerator {
using namespace godot;

void IvySettings::_bind_methods() {
	// Simulation
	ClassDB::bind_method(D_METHOD("set_seed", "seed"), &IvySettings::set_seed);
	ClassDB::bind_method(D_METHOD("get_seed"), &IvySettings::get_seed);
	ClassDB::bind_method(D_METHOD("set_start_count", "start_count"), &IvySettings::set_start_count);
	ClassDB::bind_method(D_METHOD("get_start_count"), &IvySettings::get_start_count);
	ClassDB::bind_method(D_METHOD("set_max_steps", "max_steps"), &IvySettings::set_max_steps);
	ClassDB::bind_method(D_METHOD("get_max_steps"), &IvySettings::get_max_steps);

	ADD_GROUP("Simulation", "");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "seed"), "set_seed", "get_seed");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "start_count", PROPERTY_HINT_RANGE, "1,64,1,or_greater"), "set_start_count", "get_start_count");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "max_steps", PROPERTY_HINT_RANGE, "1,5000,1,or_greater"), "set_max_steps", "get_max_steps");

	// Growth geometry
	ClassDB::bind_method(D_METHOD("set_initial_thickness", "initial_thickness"), &IvySettings::set_initial_thickness);
	ClassDB::bind_method(D_METHOD("get_initial_thickness"), &IvySettings::get_initial_thickness);
	ClassDB::bind_method(D_METHOD("set_step_length", "step_length"), &IvySettings::set_step_length);
	ClassDB::bind_method(D_METHOD("get_step_length"), &IvySettings::get_step_length);
	ClassDB::bind_method(D_METHOD("set_max_points", "max_points"), &IvySettings::set_max_points);
	ClassDB::bind_method(D_METHOD("get_max_points"), &IvySettings::get_max_points);

	ADD_GROUP("Geometry", "");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "initial_thickness", PROPERTY_HINT_RANGE, "0.001,2.0,0.001,or_greater"), "set_initial_thickness", "get_initial_thickness");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "step_length", PROPERTY_HINT_RANGE, "0.001,2.0,0.001,or_greater"), "set_step_length", "get_step_length");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "max_points", PROPERTY_HINT_RANGE, "1,100000,1,or_greater"), "set_max_points", "get_max_points");

	// Steering
	ClassDB::bind_method(D_METHOD("set_momentum_weight", "momentum_weight"), &IvySettings::set_momentum_weight);
	ClassDB::bind_method(D_METHOD("get_momentum_weight"), &IvySettings::get_momentum_weight);
	ClassDB::bind_method(D_METHOD("set_upward_weight", "upward_weight"), &IvySettings::set_upward_weight);
	ClassDB::bind_method(D_METHOD("get_upward_weight"), &IvySettings::get_upward_weight);
	ClassDB::bind_method(D_METHOD("set_random_weight", "random_weight"), &IvySettings::set_random_weight);
	ClassDB::bind_method(D_METHOD("get_random_weight"), &IvySettings::get_random_weight);
	ClassDB::bind_method(D_METHOD("set_attraction_weight", "attraction_weight"), &IvySettings::set_attraction_weight);
	ClassDB::bind_method(D_METHOD("get_attraction_weight"), &IvySettings::get_attraction_weight);
	ClassDB::bind_method(D_METHOD("set_contour_weight", "contour_weight"), &IvySettings::set_contour_weight);
	ClassDB::bind_method(D_METHOD("get_contour_weight"), &IvySettings::get_contour_weight);
	ClassDB::bind_method(D_METHOD("set_gradient_epsilon", "gradient_epsilon"), &IvySettings::set_gradient_epsilon);
	ClassDB::bind_method(D_METHOD("get_gradient_epsilon"), &IvySettings::get_gradient_epsilon);
	ClassDB::bind_method(D_METHOD("set_gradient_saturation", "gradient_saturation"), &IvySettings::set_gradient_saturation);
	ClassDB::bind_method(D_METHOD("get_gradient_saturation"), &IvySettings::get_gradient_saturation);

	ADD_GROUP("Steering", "");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "gradient_saturation", PROPERTY_HINT_RANGE, "0.0,2.0,0.01,or_greater"), "set_gradient_saturation", "get_gradient_saturation");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "momentum_weight", PROPERTY_HINT_RANGE, "0.0,10.0,0.01,or_greater"), "set_momentum_weight", "get_momentum_weight");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "upward_weight", PROPERTY_HINT_RANGE, "0.0,10.0,0.01,or_greater"), "set_upward_weight", "get_upward_weight");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "random_weight", PROPERTY_HINT_RANGE, "0.0,10.0,0.01,or_greater"), "set_random_weight", "get_random_weight");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "attraction_weight", PROPERTY_HINT_RANGE, "0.0,10.0,0.01,or_greater"), "set_attraction_weight", "get_attraction_weight");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "contour_weight", PROPERTY_HINT_RANGE, "0.0,10.0,0.01,or_greater"), "set_contour_weight", "get_contour_weight");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "gradient_epsilon", PROPERTY_HINT_RANGE, "0.0001,0.25,0.0001"), "set_gradient_epsilon", "get_gradient_epsilon");
	// Branching
	ClassDB::bind_method(D_METHOD("set_min_branch_interval", "min_branch_interval"), &IvySettings::set_min_branch_interval);
	ClassDB::bind_method(D_METHOD("get_min_branch_interval"), &IvySettings::get_min_branch_interval);
	ClassDB::bind_method(D_METHOD("set_branch_probability", "branch_probability"), &IvySettings::set_branch_probability);
	ClassDB::bind_method(D_METHOD("get_branch_probability"), &IvySettings::get_branch_probability);
	ClassDB::bind_method(D_METHOD("set_branch_angle_degrees", "branch_angle_degrees"), &IvySettings::set_branch_angle_degrees);
	ClassDB::bind_method(D_METHOD("get_branch_angle_degrees"), &IvySettings::get_branch_angle_degrees);

	ADD_GROUP("Branching", "");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "branch_probability", PROPERTY_HINT_RANGE, "0.0,1.0,0.01"), "set_branch_probability", "get_branch_probability");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "branch_angle_degrees", PROPERTY_HINT_RANGE, "1.0,90.0,0.5"), "set_branch_angle_degrees", "get_branch_angle_degrees");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "min_branch_interval", PROPERTY_HINT_RANGE, "0,50,1,or_greater"), "set_min_branch_interval", "get_min_branch_interval");

	// Thickening & Withering
	ClassDB::bind_method(D_METHOD("set_thickness_growth_per_step", "thickness_growth_per_step"), &IvySettings::set_thickness_growth_per_step);
	ClassDB::bind_method(D_METHOD("get_thickness_growth_per_step"), &IvySettings::get_thickness_growth_per_step);
	ClassDB::bind_method(D_METHOD("set_max_thickness", "max_thickness"), &IvySettings::set_max_thickness);
	ClassDB::bind_method(D_METHOD("get_max_thickness"), &IvySettings::get_max_thickness);
	ClassDB::bind_method(D_METHOD("set_wither_rate_per_step", "wither_rate_per_step"), &IvySettings::set_wither_rate_per_step);
	ClassDB::bind_method(D_METHOD("get_wither_rate_per_step"), &IvySettings::get_wither_rate_per_step);
	ClassDB::bind_method(D_METHOD("set_min_withered_thickness", "min_withered_thickness"), &IvySettings::set_min_withered_thickness);
	ClassDB::bind_method(D_METHOD("get_min_withered_thickness"), &IvySettings::get_min_withered_thickness);

	ADD_GROUP("Thickening & Withering", "");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "thickness_growth_per_step", PROPERTY_HINT_RANGE, "0.0,0.5,0.001,or_greater"), "set_thickness_growth_per_step", "get_thickness_growth_per_step");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "max_thickness", PROPERTY_HINT_RANGE, "0.001,5.0,0.001,or_greater"), "set_max_thickness", "get_max_thickness");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "wither_rate_per_step", PROPERTY_HINT_RANGE, "0.0,0.5,0.001,or_greater"), "set_wither_rate_per_step", "get_wither_rate_per_step");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "min_withered_thickness", PROPERTY_HINT_RANGE, "0.001,2.0,0.001,or_greater"), "set_min_withered_thickness", "get_min_withered_thickness");

	// Crowding & Avoidance
	ClassDB::bind_method(D_METHOD("set_crowding_radius", "crowding_radius"), &IvySettings::set_crowding_radius);
	ClassDB::bind_method(D_METHOD("get_crowding_radius"), &IvySettings::get_crowding_radius);
	ClassDB::bind_method(D_METHOD("set_avoidance_weight", "avoidance_weight"), &IvySettings::set_avoidance_weight);
	ClassDB::bind_method(D_METHOD("get_avoidance_weight"), &IvySettings::get_avoidance_weight);
	ClassDB::bind_method(D_METHOD("set_max_crowding_neighbors", "max_crowding_neighbors"), &IvySettings::set_max_crowding_neighbors);
	ClassDB::bind_method(D_METHOD("get_max_crowding_neighbors"), &IvySettings::get_max_crowding_neighbors);

	ADD_GROUP("Crowding & Avoidance", "");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "crowding_radius", PROPERTY_HINT_RANGE, "0.0,2.0,0.01,or_greater"), "set_crowding_radius", "get_crowding_radius");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "avoidance_weight", PROPERTY_HINT_RANGE, "0.0,10.0,0.01,or_greater"), "set_avoidance_weight", "get_avoidance_weight");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "max_crowding_neighbors", PROPERTY_HINT_RANGE, "0,32,1,or_greater"), "set_max_crowding_neighbors", "get_max_crowding_neighbors");

	// Targets
	ClassDB::bind_method(D_METHOD("set_target_weight", "target_weight"), &IvySettings::set_target_weight);
	ClassDB::bind_method(D_METHOD("get_target_weight"), &IvySettings::get_target_weight);
	ClassDB::bind_method(D_METHOD("set_target_influence_radius", "target_influence_radius"), &IvySettings::set_target_influence_radius);
	ClassDB::bind_method(D_METHOD("get_target_influence_radius"), &IvySettings::get_target_influence_radius);
	ClassDB::bind_method(D_METHOD("set_target_acceptance_radius", "target_acceptance_radius"), &IvySettings::set_target_acceptance_radius);
	ClassDB::bind_method(D_METHOD("get_target_acceptance_radius"), &IvySettings::get_target_acceptance_radius);

	ADD_GROUP("Targets", "");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "target_weight", PROPERTY_HINT_RANGE, "0.0,10.0,0.01,or_greater"), "set_target_weight", "get_target_weight");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "target_influence_radius", PROPERTY_HINT_RANGE, "0.0,5.0,0.01,or_greater"), "set_target_influence_radius", "get_target_influence_radius");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "target_acceptance_radius", PROPERTY_HINT_RANGE, "0.01,2.0,0.01,or_greater"), "set_target_acceptance_radius", "get_target_acceptance_radius");
}

void IvySettings::set_seed(int32_t seed) { m_seed = seed; }
int32_t IvySettings::get_seed() const { return m_seed; }

void IvySettings::set_start_count(int32_t start_count) { m_start_count = start_count; }
int32_t IvySettings::get_start_count() const { return m_start_count; }

void IvySettings::set_max_steps(int32_t max_steps) { m_max_steps = max_steps; }
int32_t IvySettings::get_max_steps() const { return m_max_steps; }

void IvySettings::set_initial_thickness(float initial_thickness) { m_initial_thickness = initial_thickness; }
float IvySettings::get_initial_thickness() const { return m_initial_thickness; }

void IvySettings::set_step_length(float step_length) { m_step_length = step_length; }
float IvySettings::get_step_length() const { return m_step_length; }

void IvySettings::set_max_points(int32_t max_points) { m_max_points = max_points; }
int32_t IvySettings::get_max_points() const { return m_max_points; }

void IvySettings::set_momentum_weight(float momentum_weight) { m_momentum_weight = momentum_weight; }
float IvySettings::get_momentum_weight() const { return m_momentum_weight; }

void IvySettings::set_upward_weight(float upward_weight) { m_upward_weight = upward_weight; }
float IvySettings::get_upward_weight() const { return m_upward_weight; }

void IvySettings::set_random_weight(float random_weight) { m_random_weight = random_weight; }
float IvySettings::get_random_weight() const { return m_random_weight; }

void IvySettings::set_attraction_weight(float attraction_weight) { m_attraction_weight = attraction_weight; }
float IvySettings::get_attraction_weight() const { return m_attraction_weight; }

void IvySettings::set_contour_weight(float contour_weight) { m_contour_weight = contour_weight; }
float IvySettings::get_contour_weight() const { return m_contour_weight; }

void IvySettings::set_gradient_epsilon(float gradient_epsilon) { m_gradient_epsilon = gradient_epsilon; }
float IvySettings::get_gradient_epsilon() const { return m_gradient_epsilon; }

void IvySettings::set_branch_probability(float branch_probability) { m_branch_probability = branch_probability; }
float IvySettings::get_branch_probability() const { return m_branch_probability; }

void IvySettings::set_branch_angle_degrees(float branch_angle_degrees) { m_branch_angle_degrees = branch_angle_degrees; }
float IvySettings::get_branch_angle_degrees() const { return m_branch_angle_degrees; }

void IvySettings::set_min_branch_interval(int32_t min_branch_interval) { m_min_branch_interval = min_branch_interval; }
int32_t IvySettings::get_min_branch_interval() const { return m_min_branch_interval; }

void IvySettings::set_thickness_growth_per_step(float thickness_growth_per_step) { m_thickness_growth_per_step = thickness_growth_per_step; }
float IvySettings::get_thickness_growth_per_step() const { return m_thickness_growth_per_step; }

void IvySettings::set_max_thickness(float max_thickness) { m_max_thickness = max_thickness; }
float IvySettings::get_max_thickness() const { return m_max_thickness; }

void IvySettings::set_wither_rate_per_step(float wither_rate_per_step) { m_wither_rate_per_step = wither_rate_per_step; }
float IvySettings::get_wither_rate_per_step() const { return m_wither_rate_per_step; }

void IvySettings::set_min_withered_thickness(float min_withered_thickness) { m_min_withered_thickness = min_withered_thickness; }
float IvySettings::get_min_withered_thickness() const { return m_min_withered_thickness; }

void IvySettings::set_crowding_radius(float crowding_radius) { m_crowding_radius = crowding_radius; }
float IvySettings::get_crowding_radius() const { return m_crowding_radius; }

void IvySettings::set_avoidance_weight(float avoidance_weight) { m_avoidance_weight = avoidance_weight; }
float IvySettings::get_avoidance_weight() const { return m_avoidance_weight; }

void IvySettings::set_max_crowding_neighbors(int32_t max_crowding_neighbours) { m_max_crowding_neighbors = max_crowding_neighbours; }
int32_t IvySettings::get_max_crowding_neighbors() const { return m_max_crowding_neighbors; }

void IvySettings::set_gradient_saturation(float gradient_saturation) { m_gradient_saturation = gradient_saturation; }
float IvySettings::get_gradient_saturation() const { return m_gradient_saturation; }

void IvySettings::set_target_weight(float target_weight) { m_target_weight = target_weight; }
float IvySettings::get_target_weight() const { return m_target_weight; }

void IvySettings::set_target_influence_radius(float target_influence_radius) { m_target_influence_radius = target_influence_radius; }
float IvySettings::get_target_influence_radius() const { return m_target_influence_radius; }

void IvySettings::set_target_acceptance_radius(float target_acceptance_radius) { m_target_acceptance_radius = target_acceptance_radius; }
float IvySettings::get_target_acceptance_radius() const { return m_target_acceptance_radius; }

IvyGrowthConfig IvySettings::to_growth_config() const {
	constexpr float deg_to_rad = 3.14159265358979323846f / 180.0f;

	IvyGrowthConfig config;
	config.initial_thickness = m_initial_thickness;
	config.step_length = m_step_length;
	config.momentum_weight = m_momentum_weight;
	config.upward_weight = m_upward_weight;
	config.random_weight = m_random_weight;
	config.attraction_weight = m_attraction_weight;
	config.contour_weight = m_contour_weight;
	config.gradient_epsilon = m_gradient_epsilon;
	config.branch_probability = m_branch_probability;
	config.branch_angle = m_branch_angle_degrees * deg_to_rad;
	config.max_points = m_max_points;
	config.min_branch_interval = m_min_branch_interval;
	config.thickness_growth_per_step = m_thickness_growth_per_step;
	config.max_thickness = m_max_thickness;
	config.wither_rate_per_step = m_wither_rate_per_step;
	config.min_withered_thickness = m_min_withered_thickness;
	config.gradient_saturation = m_gradient_saturation;
	config.crowding_radius = m_crowding_radius;
	config.avoidance_weight = m_avoidance_weight;
	config.max_crowding_neighbors = m_max_crowding_neighbors;
	config.target_weight = m_target_weight;
	config.target_influence_radius = m_target_influence_radius;
	config.target_acceptance_radius = m_target_acceptance_radius;
	return config;
}

} // namespace ivygenerator