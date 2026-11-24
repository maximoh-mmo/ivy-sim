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

	if (!std::isfinite(config.thickness_growth_per_step) || config.thickness_growth_per_step < 0.0f ||
			!std::isfinite(config.max_thickness) || config.max_thickness < config.initial_thickness ||
			!std::isfinite(config.wither_rate_per_step) || config.wither_rate_per_step < 0.0f ||
			!std::isfinite(config.min_withered_thickness) ||
			config.min_withered_thickness <= 0.0f ||
			config.min_withered_thickness > config.initial_thickness) {
		return false;
	}

	if (!std::isfinite(config.step_length) || config.step_length <= 0.0f) {
		return false;
	}

	if (!std::isfinite(config.momentum_weight) || config.momentum_weight < 0.0f ||
			!std::isfinite(config.upward_weight) || config.upward_weight < 0.0f ||
			!std::isfinite(config.random_weight) || config.random_weight < 0.0f ||
			!std::isfinite(config.attraction_weight) || config.attraction_weight < 0.0f ||
			!std::isfinite(config.contour_weight) || config.contour_weight < 0.0f) {
		return false;
	}

	if (config.momentum_weight + config.upward_weight <= 0.0f) {
		return false;
	}

	if (!std::isfinite(config.gradient_epsilon) || config.gradient_epsilon <= 0.0f) {
		return false;
	}

	if (start_points.empty()) {
		return false;
	}

	if (!std::isfinite(config.branch_probability) ||
			config.branch_probability < 0.0f || config.branch_probability > 1.0f) {
		return false;
	}

	constexpr float half_pi_upper = 1.5707964f;
	if (!std::isfinite(config.branch_angle) ||
			config.branch_angle <= 0.0f || config.branch_angle > half_pi_upper) {
		return false;
	}

	if (config.max_points <= 0) {
		return false;
	}

	if (static_cast<int64_t>(start_points.size()) > config.max_points) {
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

	if (m_graph.get_point_count() >= m_config->max_points) {
		m_active_tips.clear();
		return GrowthStepResult::FINISHED;
	}

	std::vector<int32_t> next_active_tips;
	next_active_tips.reserve(m_active_tips.size());

	for (const int32_t tip_index : m_active_tips) {
		if (m_graph.get_point_count() >= m_config->max_points) {
			break;
		}

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
		next_point.thickness = m_config->initial_thickness;
		next_point.parent_index = tip_index;

		const int32_t next_index = m_graph.add_point(next_point);
		if (next_index < 0) {
			return GrowthStepResult::ERROR;
		}

		next_active_tips.push_back(next_index);

		if (!try_spawn_branch(tip_index, current_local, direction_local, next_active_tips)) {
			return GrowthStepResult::ERROR;
		}
	}

	if (!update_thicknesses(next_active_tips)) {
		return GrowthStepResult::ERROR;
	}
	if (m_graph.get_point_count() >= m_config->max_points) {
		m_active_tips.clear();
	} else {
		m_active_tips = std::move(next_active_tips);
	}

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

bool IvyGrowthSolver::compute_step_local_direction(
		const IvyPoint &tip,
		const godot::Vector2 &tip_local,
		godot::Vector2 &out_direction) {
	godot::Vector2 previous_direction;
	if (!compute_previous_local_direction(tip, tip_local, previous_direction)) {
		return false;
	}

	const godot::Vector2 upward_direction(0.0f, 1.0f);

	godot::Vector2 combined =
			previous_direction * m_config->momentum_weight +
			upward_direction * m_config->upward_weight;

	const godot::Vector2 reference_direction =
			(combined.length_squared() > 0.000001f) ? combined : previous_direction;

	godot::Vector2 attraction_direction;
	godot::Vector2 contour_direction;
	compute_attraction_vectors(
			tip_local,
			reference_direction,
			attraction_direction,
			contour_direction);

	combined += attraction_direction * m_config->attraction_weight +
			contour_direction * m_config->contour_weight;

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

void IvyGrowthSolver::compute_attraction_vectors(
		const godot::Vector2 &tip_local,
		const godot::Vector2 &reference_direction,
		godot::Vector2 &out_attraction_direction,
		godot::Vector2 &out_contour_direction) const {
	out_attraction_direction = godot::Vector2();
	out_contour_direction = godot::Vector2();

	if (m_config->attraction_weight <= 0.0f && m_config->contour_weight <= 0.0f) {
		return;
	}

	const godot::Vector2 uv = m_wall->local_to_uv(tip_local);
	const float eps = m_config->gradient_epsilon;

	const float attr_u_plus =
			m_surface_sampler->sample_attraction(godot::Vector2(uv.x + eps, uv.y));
	const float attr_u_minus =
			m_surface_sampler->sample_attraction(godot::Vector2(uv.x - eps, uv.y));
	const float attr_v_plus =
			m_surface_sampler->sample_attraction(godot::Vector2(uv.x, uv.y + eps));
	const float attr_v_minus =
			m_surface_sampler->sample_attraction(godot::Vector2(uv.x, uv.y - eps));

	const float inv_two_eps = 1.0f / (2.0f * eps);
	const float grad_u = (attr_u_plus - attr_u_minus) * inv_two_eps;
	const float grad_v = (attr_v_plus - attr_v_minus) * inv_two_eps;

	const godot::Vector2 physical_gradient(
			grad_u * (m_wall->uv_scale.x / m_wall->width),
			grad_v * (m_wall->uv_scale.y / m_wall->height));

	if (physical_gradient.length_squared() <= 0.00000001f) {
		return;
	}

	out_attraction_direction = physical_gradient.normalized();

	const godot::Vector2 tangent(
			-out_attraction_direction.y,
			out_attraction_direction.x);
	out_contour_direction =
			(tangent.dot(reference_direction) >= 0.0f) ? tangent : -tangent;
}

bool IvyGrowthSolver::try_spawn_branch(
		int32_t parent_index,
		const godot::Vector2 &parent_local,
		const godot::Vector2 &main_direction_local,
		std::vector<int32_t> &next_active_tips) {
	if (m_config->branch_probability <= 0.0f) {
		return true;
	}

	if (m_graph.get_point_count() >= m_config->max_points) {
		return true;
	}

	if (m_random->random_float() >= m_config->branch_probability) {
		return true;
	}

	const float branch_sign = m_random->random_bool() ? 1.0f : -1.0f;
	const godot::Vector2 branch_direction =
			main_direction_local.rotated(branch_sign * m_config->branch_angle);
	const godot::Vector2 branch_local =
			parent_local + branch_direction * m_config->step_length;

	if (!m_wall->contains_local_point(branch_local)) {
		return true;
	}

	IvyPoint branch_point;
	branch_point.position = m_wall->local_to_world(branch_local);
	branch_point.thickness = m_config->initial_thickness;
	branch_point.parent_index = parent_index;

	const int32_t branch_index = m_graph.add_point(branch_point);
	if (branch_index < 0) {
		return false;
	}

	next_active_tips.push_back(branch_index);
	return true;
}

bool IvyGrowthSolver::update_thicknesses(const std::vector<int32_t> &active_tips) {
	if (active_tips.empty()) {
		return true;
	}

	if (m_config->thickness_growth_per_step <= 0.0f && m_config->wither_rate_per_step <= 0.0f) {
		return true;
	}

	const int32_t point_count = m_graph.get_point_count();
	std::vector<int32_t> downstream_tips(static_cast<size_t>(point_count), 0);
	std::vector<uint8_t> is_active_tip(static_cast<size_t>(point_count), 0);

	for (const int32_t tip_index : active_tips) {
		if (tip_index < 0 || tip_index >= point_count) {
			return false;
		}
		downstream_tips[static_cast<size_t>(tip_index)] += 1;
		is_active_tip[static_cast<size_t>(tip_index)] = 1;
	}

	// Accumulate downstream active-tip support upward toward roots in O(N).
	for (int32_t i = point_count - 1; i >= 0; --i) {
		const int32_t support = downstream_tips[static_cast<size_t>(i)];
		if (support <= 0) {
			continue;
		}

		const IvyPoint *point = m_graph.get_point(i);
		if (point == nullptr) {
			return false;
		}

		if (point->parent_index >= 0 && point->parent_index < point_count) {
			downstream_tips[static_cast<size_t>(point->parent_index)] += support;
		}
	}

	for (int32_t i = 0; i < point_count; ++i) {
		if (is_active_tip[static_cast<size_t>(i)] != 0) {
			continue;
		}

		const IvyPoint *point = m_graph.get_point(i);
		if (point == nullptr) {
			return false;
		}

		const int32_t support = downstream_tips[static_cast<size_t>(i)];
		float updated_thickness = point->thickness;

		if (support > 0) {
			if (m_config->thickness_growth_per_step > 0.0f) {
				const float growth = m_config->thickness_growth_per_step *
						std::sqrt(static_cast<float>(support));
				updated_thickness = std::min(m_config->max_thickness, point->thickness + growth);
			}
		} else {
			if (m_config->wither_rate_per_step > 0.0f) {
				updated_thickness = std::max(
						m_config->min_withered_thickness,
						point->thickness - m_config->wither_rate_per_step);
			}
		}

		if (updated_thickness != point->thickness) {
			if (!m_graph.set_point_thickness(i, updated_thickness)) {
				return false;
			}
		}
	}

	return true;
}

} // namespace ivygenerator
