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
#include <algorithm>
#include <cmath>

namespace ivygenerator {
bool IvyGrowthSolver::initialize(
		const IvyWall &wall,
		const IvySurfaceSampler &surface_sampler,
		const IvyGrowthConfig &config,
		IvyRandom &random,
		const std::vector<godot::Vector3> &start_points,
		const std::vector<godot::Vector3> &target_points) {
	m_graph.clear();
	m_active_tips.clear();
	m_steps_since_branch.clear();
	m_wall = nullptr;
	m_surface_sampler = nullptr;
	m_config = nullptr;
	m_random = nullptr;
	m_local_positions.clear();
	m_grid_heads.clear();
	m_grid_next.clear();
	m_grid_cols = 0;
	m_grid_rows = 0;
	m_grid_cell_size = 0.0f;
	m_target_points_local.clear();
	m_target_reached.clear();

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

	if (!std::isfinite(config.gradient_saturation) || config.gradient_saturation < 0.0f) {
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

	if (config.min_branch_interval < 0) {
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

	if (!std::isfinite(config.crowding_radius) || config.crowding_radius < 0.0f ||
			!std::isfinite(config.avoidance_weight) || config.avoidance_weight < 0.0f ||
			config.max_crowding_neighbors < 0) {
		return false;
	}

	if (!std::isfinite(config.target_weight) || config.target_weight < 0.0f ||
			!std::isfinite(config.target_influence_radius) || config.target_influence_radius < 0.0f ||
			!std::isfinite(config.target_acceptance_radius) || config.target_acceptance_radius <= 0.0f) {
		return false;
	}

	for (const godot::Vector3 &target_point : target_points) {
		if (!wall.is_on_surface(target_point)) {
			return false;
		}
	}

	if (config.crowding_radius > 0.0f) {
		const float max_extent = std::max(wall.width, wall.height);
		m_grid_cell_size = std::max(config.crowding_radius, max_extent / 256.0f);
		m_grid_cols = std::max(1, static_cast<int32_t>(std::ceil(wall.width / m_grid_cell_size)));
		m_grid_rows = std::max(1, static_cast<int32_t>(std::ceil(wall.height / m_grid_cell_size)));
		m_grid_heads.assign(
				static_cast<size_t>(m_grid_cols) * static_cast<size_t>(m_grid_rows), -1);
	}
	m_active_tips.reserve(start_points.size());
	m_steps_since_branch.reserve(start_points.size());
	for (const godot::Vector3 &start_point : start_points) {
		IvyPoint root_point;
		root_point.position = start_point;
		root_point.thickness = config.initial_thickness;
		root_point.parent_index = -1;
		const int32_t root_index = m_graph.add_point(root_point);
		if (root_index < 0) {
			m_graph.clear();
			m_active_tips.clear();
			m_steps_since_branch.clear();
			m_local_positions.clear();
			m_grid_heads.clear();
			m_grid_next.clear();
			m_target_points_local.clear();
			m_target_reached.clear();
			return false;
		}
		m_active_tips.push_back(root_index);
		m_steps_since_branch.push_back(0);
		register_point_in_grid(root_index, wall.world_to_local(start_point));
	}

	m_target_points_local.reserve(target_points.size());
	for (const godot::Vector3 &target_point : target_points) {
		m_target_points_local.push_back(wall.world_to_local(target_point));
	}
	m_target_reached.assign(target_points.size(), 0);

	m_wall = &wall;
	m_surface_sampler = &surface_sampler;
	m_config = &config;
	m_random = &random;

	for (const godot::Vector2 &root_local : m_local_positions) {
		update_reached_targets(root_local, root_local);
	}

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
		if (!compute_step_local_direction(tip_index, tip, current_local, direction_local)) {
			return GrowthStepResult::ERROR;
		}

		const godot::Vector2 next_local =
				current_local + direction_local * m_config->step_length;

		if (!m_wall->contains_local_point(next_local)) {
			// Tip stepped outside the wall boundary and terminates.
			continue;
		}

		if (m_config->max_crowding_neighbors > 0 &&
				count_crowding_neighbors(tip_index, next_local) >= m_config->max_crowding_neighbors) {
			continue;
		}

		IvyPoint next_point;
		next_point.position = m_wall->local_to_world(next_local);
		next_point.thickness = m_config->initial_thickness;
		next_point.parent_index = tip_index;

		const int32_t tip_steps_since_branch =
				m_steps_since_branch[static_cast<size_t>(tip_index)];

		const int32_t next_index = m_graph.add_point(next_point);
		if (next_index < 0) {
			return GrowthStepResult::ERROR;
		}

		m_steps_since_branch.push_back(tip_steps_since_branch + 1);
		register_point_in_grid(next_index, next_local);
		update_reached_targets(current_local, next_local);
		next_active_tips.push_back(next_index);

		if (tip_steps_since_branch >= m_config->min_branch_interval) {
			bool spawned_branch = false;
			if (!try_spawn_branch(tip_index, current_local, direction_local, next_active_tips, spawned_branch)) {
				return GrowthStepResult::ERROR;
			}
			if (spawned_branch) {
				m_steps_since_branch[static_cast<size_t>(next_index)] = 0;
			}
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
		int32_t tip_index,
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

	if (m_config->avoidance_weight > 0.0f && m_config->crowding_radius > 0.0f) {
		combined += compute_avoidance_vector(tip_index, tip_local) * m_config->avoidance_weight;
	}
	if (m_config->target_weight > 0.0f && !m_target_points_local.empty()) {
		combined += compute_target_vector(tip_local) * m_config->target_weight;
	}
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

	const float gradient_length = physical_gradient.length();
	if (gradient_length <= 0.0001f) {
		return;
	}
	const float step_gain = gradient_length * m_config->step_length;
	const float strength = (m_config->gradient_saturation > 0.0f)
			? (step_gain / (step_gain + m_config->gradient_saturation))
			: 1.0f;
	const godot::Vector2 gradient_dir = physical_gradient / gradient_length;
	out_attraction_direction = gradient_dir * strength;
	const godot::Vector2 tangent(-gradient_dir.y, gradient_dir.x);
	const godot::Vector2 contour_dir =
			(tangent.dot(reference_direction) >= 0.0f) ? tangent : -tangent;
	out_contour_direction = contour_dir * strength;
}

bool IvyGrowthSolver::try_spawn_branch(
		int32_t parent_index,
		const godot::Vector2 &parent_local,
		const godot::Vector2 &main_direction_local,
		std::vector<int32_t> &next_active_tips,
		bool &out_branched) {
	out_branched = false;
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

	m_steps_since_branch.push_back(0);
	register_point_in_grid(branch_index, branch_local);
	update_reached_targets(parent_local, branch_local);
	next_active_tips.push_back(branch_index);
	out_branched = true;
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
void IvyGrowthSolver::register_point_in_grid(
		int32_t point_index,
		const godot::Vector2 &local_pos) {
	m_local_positions.push_back(local_pos);

	if (m_grid_heads.empty()) {
		return;
	}

	const int32_t col = std::clamp(
			static_cast<int32_t>(local_pos.x / m_grid_cell_size), 0, m_grid_cols - 1);
	const int32_t row = std::clamp(
			static_cast<int32_t>(local_pos.y / m_grid_cell_size), 0, m_grid_rows - 1);
	const int32_t cell = row * m_grid_cols + col;

	m_grid_next.push_back(m_grid_heads[static_cast<size_t>(cell)]);
	m_grid_heads[static_cast<size_t>(cell)] = point_index;
}

bool IvyGrowthSolver::is_recent_lineage(
		int32_t tip_index,
		int32_t candidate_index,
		int32_t max_hops) const {
	int32_t current = tip_index;
	for (int32_t hop = 0; hop <= max_hops && current >= 0; ++hop) {
		if (current == candidate_index) {
			return true;
		}
		const IvyPoint *pt = m_graph.get_point(current);
		if (pt == nullptr) {
			break;
		}
		current = pt->parent_index;
	}

	const IvyPoint *tip_pt = m_graph.get_point(tip_index);
	const IvyPoint *cand_pt = m_graph.get_point(candidate_index);
	if (tip_pt != nullptr && cand_pt != nullptr &&
			tip_pt->parent_index >= 0 &&
			tip_pt->parent_index == cand_pt->parent_index) {
		return true;
	}

	return false;
}

godot::Vector2 IvyGrowthSolver::compute_avoidance_vector(
		int32_t tip_index,
		const godot::Vector2 &tip_local) const {
	if (m_grid_heads.empty() || m_config->crowding_radius <= 0.0f) {
		return godot::Vector2();
	}

	const float radius = m_config->crowding_radius;
	const float radius_sq = radius * radius;
	const int32_t ignore_hops =
			static_cast<int32_t>(std::ceil(radius / m_config->step_length)) + 2;

	const int32_t center_col = std::clamp(
			static_cast<int32_t>(tip_local.x / m_grid_cell_size), 0, m_grid_cols - 1);
	const int32_t center_row = std::clamp(
			static_cast<int32_t>(tip_local.y / m_grid_cell_size), 0, m_grid_rows - 1);

	const int32_t min_col = std::max(0, center_col - 1);
	const int32_t max_col = std::min(m_grid_cols - 1, center_col + 1);
	const int32_t min_row = std::max(0, center_row - 1);
	const int32_t max_row = std::min(m_grid_rows - 1, center_row + 1);

	godot::Vector2 repulsion;

	for (int32_t row = min_row; row <= max_row; ++row) {
		for (int32_t col = min_col; col <= max_col; ++col) {
			int32_t idx = m_grid_heads[static_cast<size_t>(row * m_grid_cols + col)];
			while (idx >= 0) {
				if (!is_recent_lineage(tip_index, idx, ignore_hops)) {
					const godot::Vector2 delta = tip_local - m_local_positions[static_cast<size_t>(idx)];
					const float dist_sq = delta.length_squared();
					if (dist_sq > 0.000001f && dist_sq < radius_sq) {
						const float dist = std::sqrt(dist_sq);
						const float weight = 1.0f - (dist / radius);
						repulsion += (delta / dist) * weight;
					}
				}
				idx = m_grid_next[static_cast<size_t>(idx)];
			}
		}
	}

	const float len = repulsion.length();
	if (len <= 0.000001f) {
		return godot::Vector2();
	}

	return (repulsion / len) * (len / (len + 1.0f));
}

int32_t IvyGrowthSolver::count_crowding_neighbors(
		int32_t tip_index,
		const godot::Vector2 &query_local) const {
	if (m_grid_heads.empty() || m_config->crowding_radius <= 0.0f) {
		return 0;
	}

	const float radius = m_config->crowding_radius;
	const float radius_sq = radius * radius;
	const int32_t ignore_hops =
			static_cast<int32_t>(std::ceil(radius / m_config->step_length)) + 2;

	const int32_t center_col = std::clamp(
			static_cast<int32_t>(query_local.x / m_grid_cell_size), 0, m_grid_cols - 1);
	const int32_t center_row = std::clamp(
			static_cast<int32_t>(query_local.y / m_grid_cell_size), 0, m_grid_rows - 1);

	const int32_t min_col = std::max(0, center_col - 1);
	const int32_t max_col = std::min(m_grid_cols - 1, center_col + 1);
	const int32_t min_row = std::max(0, center_row - 1);
	const int32_t max_row = std::min(m_grid_rows - 1, center_row + 1);

	int32_t count = 0;

	for (int32_t row = min_row; row <= max_row; ++row) {
		for (int32_t col = min_col; col <= max_col; ++col) {
			int32_t idx = m_grid_heads[static_cast<size_t>(row * m_grid_cols + col)];
			while (idx >= 0) {
				if (!is_recent_lineage(tip_index, idx, ignore_hops)) {
					const godot::Vector2 delta = query_local - m_local_positions[static_cast<size_t>(idx)];
					if (delta.length_squared() <= radius_sq) {
						++count;
					}
				}
				idx = m_grid_next[static_cast<size_t>(idx)];
			}
		}
	}

	return count;
}

int32_t IvyGrowthSolver::get_target_count() const {
	return static_cast<int32_t>(m_target_points_local.size());
}

bool IvyGrowthSolver::is_target_reached(int32_t target_index) const {
	if (target_index < 0 || target_index >= static_cast<int32_t>(m_target_reached.size())) {
		return false;
	}
	return m_target_reached[static_cast<size_t>(target_index)] != 0;
}

int32_t IvyGrowthSolver::get_reached_target_count() const {
	int32_t count = 0;
	for (const uint8_t reached : m_target_reached) {
		if (reached != 0) {
			++count;
		}
	}
	return count;
}

godot::Vector2 IvyGrowthSolver::compute_target_vector(const godot::Vector2 &tip_local) const {
	if (m_config->target_weight <= 0.0f || m_target_points_local.empty()) {
		return godot::Vector2();
	}

	const bool use_radius = m_config->target_influence_radius > 0.0f;
	const float max_dist_sq = use_radius
			? (m_config->target_influence_radius * m_config->target_influence_radius)
			: 0.0f;

	bool found = false;
	float best_dist_sq = 0.0f;
	godot::Vector2 best_delta;

	for (size_t i = 0; i < m_target_points_local.size(); ++i) {
		if (m_target_reached[i] != 0) {
			continue;
		}

		const godot::Vector2 delta = m_target_points_local[i] - tip_local;
		const float dist_sq = delta.length_squared();
		if (use_radius && dist_sq > max_dist_sq) {
			continue;
		}

		if (!found || dist_sq < best_dist_sq) {
			found = true;
			best_dist_sq = dist_sq;
			best_delta = delta;
		}
	}

	if (!found || best_dist_sq <= 0.000001f) {
		return godot::Vector2();
	}

	const float dist = std::sqrt(best_dist_sq);
	const godot::Vector2 dir = best_delta / dist;
	const float strength = use_radius
			? std::clamp(1.0f - (dist / m_config->target_influence_radius), 0.0f, 1.0f)
			: 1.0f;
	return dir * strength;
}

void IvyGrowthSolver::update_reached_targets(
		const godot::Vector2 &segment_start,
		const godot::Vector2 &segment_end) {
	if (m_target_points_local.empty()) {
		return;
	}

	const float accept_sq = m_config->target_acceptance_radius * m_config->target_acceptance_radius;
	const godot::Vector2 seg = segment_end - segment_start;
	const float seg_len_sq = seg.length_squared();

	for (size_t i = 0; i < m_target_points_local.size(); ++i) {
		if (m_target_reached[i] != 0) {
			continue;
		}

		const godot::Vector2 target = m_target_points_local[i];
		godot::Vector2 closest = segment_end;
		if (seg_len_sq > 0.000001f) {
			const float t = std::clamp((target - segment_start).dot(seg) / seg_len_sq, 0.0f, 1.0f);
			closest = segment_start + seg * t;
		}

		if ((target - closest).length_squared() <= accept_sq) {
			m_target_reached[i] = 1;
		}
	}
}

bool IvyGrowthSolver::connect_unreached_targets() {
	if (m_wall == nullptr || m_surface_sampler == nullptr ||
			m_config == nullptr || m_random == nullptr) {
		return false;
	}

	if (m_target_points_local.empty()) {
		return true;
	}

	for (size_t target_idx = 0; target_idx < m_target_points_local.size(); ++target_idx) {
		if (m_target_reached[target_idx] != 0) {
			continue;
		}

		const godot::Vector2 target_local = m_target_points_local[target_idx];
		int32_t best_index = -1;
		float best_dist_sq = 0.0f;

		for (size_t i = 0; i < m_local_positions.size(); ++i) {
			const float dist_sq = (m_local_positions[i] - target_local).length_squared();
			if (best_index < 0 || dist_sq < best_dist_sq) {
				best_index = static_cast<int32_t>(i);
				best_dist_sq = dist_sq;
			}
		}

		if (best_index < 0) {
			return false;
		}

		int32_t current_index = best_index;
		godot::Vector2 current_local = m_local_positions[static_cast<size_t>(current_index)];
		const float initial_dist = (target_local - current_local).length();
		const int32_t max_spur_steps =
				static_cast<int32_t>(std::ceil(initial_dist / m_config->step_length)) * 3 + 16;

		for (int32_t spur_step = 0; spur_step < max_spur_steps && m_target_reached[target_idx] == 0; ++spur_step) {
			const godot::Vector2 to_target = target_local - current_local;
			const float dist = to_target.length();
			if (dist <= m_config->target_acceptance_radius) {
				m_target_reached[target_idx] = 1;
				break;
			}

			if (m_graph.get_point_count() >= m_config->max_points) {
				return false;
			}

			const godot::Vector2 target_dir = to_target / dist;
			godot::Vector2 dir = target_dir;

			if (dist > m_config->step_length) {
				godot::Vector2 combined = target_dir * 2.0f;

				const IvyPoint *tip_ptr = m_graph.get_point(current_index);
				if (tip_ptr == nullptr) {
					return false;
				}

				godot::Vector2 prev_dir;
				if (!compute_previous_local_direction(*tip_ptr, current_local, prev_dir)) {
					return false;
				}
				if (prev_dir.dot(target_dir) > 0.0f) {
					combined += prev_dir * 0.4f;
				}

				godot::Vector2 attraction_dir;
				godot::Vector2 contour_dir;
				compute_attraction_vectors(current_local, target_dir, attraction_dir, contour_dir);
				godot::Vector2 surface_pull =
						attraction_dir * m_config->attraction_weight +
						contour_dir * m_config->contour_weight;
				if (surface_pull.length_squared() > 0.16f) {
					surface_pull = surface_pull.normalized() * 0.4f;
				}
				combined += surface_pull;

				if (m_config->random_weight > 0.0f) {
					const float jitter_angle = m_random->random_float(-0.5f, 0.5f);
					combined += target_dir.rotated(jitter_angle) * 0.25f;
				}

				dir = combined.normalized();
			}

			const float step_dist = std::min(m_config->step_length, dist);
			godot::Vector2 next_local = current_local + dir * step_dist;

			if (!m_wall->contains_local_point(next_local)) {
				next_local = current_local + target_dir * step_dist;
				next_local.x = std::clamp(next_local.x, 0.0f, m_wall->width);
				next_local.y = std::clamp(next_local.y, 0.0f, m_wall->height);
			}

			IvyPoint spur_point;
			spur_point.position = m_wall->local_to_world(next_local);
			spur_point.thickness = m_config->initial_thickness;
			spur_point.parent_index = current_index;

			const int32_t spur_index = m_graph.add_point(spur_point);
			if (spur_index < 0) {
				return false;
			}

			m_steps_since_branch.push_back(0);
			register_point_in_grid(spur_index, next_local);
			update_reached_targets(current_local, next_local);

			current_index = spur_index;
			current_local = next_local;
		}

		if (m_target_reached[target_idx] == 0) {
			return false;
		}
	}

	return true;
}

} // namespace ivygenerator
