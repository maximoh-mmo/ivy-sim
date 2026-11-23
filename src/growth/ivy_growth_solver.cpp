/*
 * Copyright (c) 2026 Max Heinze. All rights reserved.
 *
 * Licensed for non-commercial use with attribution only.
 * Commercial use is strictly prohibited.
 */

/**
 * @file ivy_growth_solver.cpp
 * @brief Implementation of ivy branch propagation algorithms.
 */

#include "ivy_growth_solver.h"

#include <godot_cpp/variant/vector2.hpp>
#include <godot_cpp/variant/vector3.hpp>

#include "environment/ivy_surface_sampler.h"
#include "environment/ivy_wall.h"
#include "random/ivy_random.h"
#include "settings/ivy_growth_config.h"
#include <cmath>

namespace ivygenerator {
bool IvyGrowthSolver::initialize(
		const IvyWall &wall,
		const IvySurfaceSampler &surface_sampler,
		const IvyGrowthConfig &config,
		IvyRandom &random,
		const std::vector<godot::Vector3> &start_points) {
	m_graph.clear();
	m_active_tips.clear();
	m_wall = nullptr;
	m_surface_sampler = nullptr;
	m_config = nullptr;
	m_random = nullptr;
	if (!wall.is_valid()) {
		return false;
	}
	if (!surface_sampler.is_valid()) {
		return false;
	}

	if (!std::isfinite(config.initial_thickness) || config.initial_thickness <= 0.0f) {
		return false;
	}

	if (!std::isfinite(config.step_length) || config.step_length <= 0.0f) {
		return false;
	}

	if (!std::isfinite(config.momentum_weight) || config.momentum_weight < 0.0f ||
			!std::isfinite(config.upward_weight) || config.upward_weight < 0.0f ||
			!std::isfinite(config.random_weight) || config.random_weight < 0.0f) {
		return false;
	}
	if (!std::isfinite(config.momentum_weight) || config.momentum_weight < 0.0f ||
			!std::isfinite(config.upward_weight) || config.upward_weight < 0.0f ||
			!std::isfinite(config.random_weight) || config.random_weight < 0.0f) {
		return false;
	}

	if (config.momentum_weight + config.upward_weight <= 0.0f) {
		return false;
	}

	if (start_points.empty()) {
		return false;
	}
	for (const godot::Vector3 &start_point : start_points) {
		if (!wall.is_on_surface(start_point)) {
			return false;
		}
	}
	m_active_tips.reserve(start_points.size());
	for (const godot::Vector3 &start_point : start_points) {
		IvyPoint root_point;
		root_point.position = start_point;
		root_point.thickness = config.initial_thickness;
		root_point.parent_index = -1;
		const int32_t root_index = m_graph.add_point(root_point);
		if (root_index < 0) {
			m_graph.clear();
			m_active_tips.clear();
			return false;
		}
		m_active_tips.push_back(root_index);
	}
	m_wall = &wall;
	m_surface_sampler = &surface_sampler;
	m_config = &config;
	m_random = &random;
	return true;
}

GrowthStepResult IvyGrowthSolver::step() {
	if (m_wall == nullptr || m_surface_sampler == nullptr ||
			m_config == nullptr || m_random == nullptr) {
		return GrowthStepResult::ERROR;
	}
	if (m_active_tips.empty()) {
		return GrowthStepResult::FINISHED;
	}
	std::vector<int32_t> next_active_tips;
	next_active_tips.reserve(m_active_tips.size());
	for (const int32_t tip_index : m_active_tips) {
		const IvyPoint *tip_ptr = m_graph.get_point(tip_index);
		if (tip_ptr == nullptr) {
			return GrowthStepResult::ERROR;
		}
		// Copy by value in case m_graph.add_point() reallocates m_graph's internal vector.
		const IvyPoint tip = *tip_ptr;
		const godot::Vector2 current_local = m_wall->world_to_local(tip.position);
		godot::Vector2 direction_local;
		if (!compute_step_local_direction(tip, current_local, direction_local)) {
			return GrowthStepResult::ERROR;
		}
		const godot::Vector2 next_local =
				current_local + direction_local * m_config->step_length;
		if (!m_wall->contains_local_point(next_local)) {
			// Tip stepped outside the wall boundary and terminates.
			continue;
		}
		IvyPoint next_point;
		next_point.position = m_wall->local_to_world(next_local);
		next_point.thickness = tip.thickness;
		next_point.parent_index = tip_index;
		const int32_t next_index = m_graph.add_point(next_point);
		if (next_index < 0) {
			return GrowthStepResult::ERROR;
		}
		next_active_tips.push_back(next_index);
	}
	m_active_tips = std::move(next_active_tips);
	return GrowthStepResult::ADVANCED;
}

bool IvyGrowthSolver::is_finished() const {
	return m_active_tips.empty();
}

const IvyGrowthGraph &IvyGrowthSolver::get_graph() const {
	// TODO: insert return statement here
	return m_graph;
}

bool IvyGrowthSolver::compute_previous_local_direction(
		const IvyPoint &tip,
		const godot::Vector2 &tip_local,
		godot::Vector2 &out_direction) const {
	if (tip.parent_index == -1) {
		out_direction = godot::Vector2(0.0f, 1.0f);
		return true;
	}
	const IvyPoint *parent = m_graph.get_point(tip.parent_index);
	if (parent == nullptr) {
		return false;
	}
	const godot::Vector2 parent_local = m_wall->world_to_local(parent->position);
	const godot::Vector2 delta = tip_local - parent_local;
	if (delta.length_squared() <= 0.000001f) {
		out_direction = godot::Vector2(0.0f, 1.0f);
		return true;
	}
	out_direction = delta.normalized();
	return true;
}

bool IvyGrowthSolver::compute_step_local_direction(const IvyPoint &tip, const godot::Vector2 &tip_local, godot::Vector2 &out_direction) {
	godot::Vector2 previous_direction;
	if (!compute_previous_local_direction(tip, tip_local, previous_direction)) {
		return false;
	}
	const godot::Vector2 upward_direction(0.0f, 1.0f);
	godot::Vector2 combined =
			previous_direction * m_config->momentum_weight +
			upward_direction * m_config->upward_weight;
	if (m_config->random_weight > 0.0f) {
		constexpr float half_pi = 1.57079632679f;
		const float random_angle = m_random->random_float(-half_pi, half_pi);
		const godot::Vector2 random_direction = previous_direction.rotated(random_angle);
		combined += random_direction * m_config->random_weight;
	}
	if (combined.length_squared() <= 0.000001f) {
		out_direction = previous_direction;
		return true;
	}
	out_direction = combined.normalized();
	return true;
}
} //namespace ivygenerator