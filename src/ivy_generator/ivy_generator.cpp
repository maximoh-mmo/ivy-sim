/*
 * Copyright (c) 2026 Max Heinze. All rights reserved.
 *
 * Licensed for non-commercial use with attribution only.
 * Commercial use is strictly prohibited.
 */

/**
 * @file ivy_generator.cpp
 * @brief Implementation of IvyGenerator RefCounted lifecycle and generation routines.
 */

#include "ivy_generator.h"

#include <godot_cpp/core/class_db.hpp>

#include <vector>

namespace ivygenerator {
using namespace godot;

void IvyGenerator::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_settings", "settings"), &IvyGenerator::set_settings);
	ClassDB::bind_method(D_METHOD("get_settings"), &IvyGenerator::get_settings);
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "settings", PROPERTY_HINT_RESOURCE_TYPE, "IvySettings"),
			"set_settings", "get_settings");

	ClassDB::bind_method(
			D_METHOD("set_wall", "origin", "axis_u", "axis_v", "width", "height", "uv_scale", "uv_offset"),
			&IvyGenerator::set_wall,
			DEFVAL(Vector2(1.0f, 1.0f)),
			DEFVAL(Vector2(0.0f, 0.0f)));

	ClassDB::bind_method(D_METHOD("set_attraction_image", "image"), &IvyGenerator::set_attraction_image);

	ClassDB::bind_method(D_METHOD("set_start_points", "start_points"), &IvyGenerator::set_start_points);
	ClassDB::bind_method(D_METHOD("get_start_points"), &IvyGenerator::get_start_points);
	ClassDB::bind_method(D_METHOD("clear_start_points"), &IvyGenerator::clear_start_points);

	ClassDB::bind_method(D_METHOD("set_target_points", "target_points"), &IvyGenerator::set_target_points);
	ClassDB::bind_method(D_METHOD("get_target_points"), &IvyGenerator::get_target_points);
	ClassDB::bind_method(D_METHOD("clear_target_points"), &IvyGenerator::clear_target_points);

	ClassDB::bind_method(D_METHOD("generate"), &IvyGenerator::generate);

	ClassDB::bind_method(D_METHOD("get_point_count"), &IvyGenerator::get_point_count);
	ClassDB::bind_method(D_METHOD("get_point_positions"), &IvyGenerator::get_point_positions);
	ClassDB::bind_method(D_METHOD("get_point_thicknesses"), &IvyGenerator::get_point_thicknesses);
	ClassDB::bind_method(D_METHOD("get_point_parent_indices"), &IvyGenerator::get_point_parent_indices);

	ClassDB::bind_method(D_METHOD("get_target_count"), &IvyGenerator::get_target_count);
	ClassDB::bind_method(D_METHOD("is_target_reached", "target_index"), &IvyGenerator::is_target_reached);
	ClassDB::bind_method(D_METHOD("get_reached_target_count"), &IvyGenerator::get_reached_target_count);
}

void IvyGenerator::set_settings(const Ref<IvySettings> &settings) {
	m_settings = settings;
}

Ref<IvySettings> IvyGenerator::get_settings() const {
	return m_settings;
}

bool IvyGenerator::set_wall(
		const Vector3 &origin,
		const Vector3 &axis_u,
		const Vector3 &axis_v,
		float width,
		float height,
		const Vector2 &uv_scale,
		const Vector2 &uv_offset) {
	IvyWall candidate;
	candidate.origin = origin;
	candidate.axis_u = axis_u;
	candidate.axis_v = axis_v;
	candidate.width = width;
	candidate.height = height;
	candidate.uv_scale = uv_scale;
	candidate.uv_offset = uv_offset;

	if (!candidate.is_valid()) {
		return false;
	}

	m_wall = candidate;
	return true;
}

bool IvyGenerator::set_attraction_image(const Ref<Image> &image) {
	return m_surface_sampler.load_image(image);
}

void IvyGenerator::set_start_points(const PackedVector3Array &start_points) {
	m_explicit_start_points = start_points;
}

PackedVector3Array IvyGenerator::get_start_points() const {
	return m_explicit_start_points;
}

void IvyGenerator::clear_start_points() {
	m_explicit_start_points.clear();
}

void IvyGenerator::set_target_points(const PackedVector3Array &target_points) {
	m_target_points = target_points;
}

PackedVector3Array IvyGenerator::get_target_points() const {
	return m_target_points;
}

void IvyGenerator::clear_target_points() {
	m_target_points.clear();
}

void IvyGenerator::clear_results() {
	m_positions.clear();
	m_thicknesses.clear();
	m_parent_indices.clear();
	m_target_reached.clear();
}

bool IvyGenerator::generate() {
	clear_results();

	if (m_settings.is_null()) {
		return false;
	}

	if (!m_wall.is_valid() || !m_surface_sampler.is_valid()) {
		return false;
	}

	const int32_t start_count = m_settings->get_start_count();
	const int32_t max_steps = m_settings->get_max_steps();
	if (start_count <= 0 || max_steps <= 0) {
		return false;
	}

	const int64_t explicit_count = m_explicit_start_points.size();
	if (explicit_count > static_cast<int64_t>(start_count)) {
		return false;
	}

	m_random.set_seed(m_settings->get_seed());

	std::vector<Vector3> resolved_starts;
	resolved_starts.reserve(static_cast<size_t>(start_count));

	for (int64_t i = 0; i < explicit_count; ++i) {
		const Vector3 point = m_explicit_start_points[i];
		if (!m_wall.is_on_surface(point)) {
			return false;
		}
		resolved_starts.push_back(point);
	}

	const int32_t generated_count = start_count - static_cast<int32_t>(explicit_count);
	for (int32_t i = 0; i < generated_count; ++i) {
		const float u = m_random.random_float(0.0f, m_wall.width);
		resolved_starts.push_back(m_wall.local_to_world(Vector2(u, 0.0f)));
	}

	std::vector<Vector3> resolved_targets;
	resolved_targets.reserve(static_cast<size_t>(m_target_points.size()));
	for (int64_t i = 0; i < m_target_points.size(); ++i) {
		const Vector3 target = m_target_points[i];
		if (!m_wall.is_on_surface(target)) {
			return false;
		}
		resolved_targets.push_back(target);
	}

	m_growth_config = m_settings->to_growth_config();

	if (!m_solver.initialize(
				m_wall,
				m_surface_sampler,
				m_growth_config,
				m_random,
				resolved_starts,
				resolved_targets)) {
		return false;
	}

	for (int32_t step_index = 0; step_index < max_steps; ++step_index) {
		const GrowthStepResult result = m_solver.step();
		if (result == GrowthStepResult::FINISHED) {
			break;
		}
		if (result == GrowthStepResult::ERROR) {
			return false;
		}
	}

	if (!m_solver.connect_unreached_targets()) {
		return false;
	}

	const IvyGrowthGraph &graph = m_solver.get_graph();
	const int32_t point_count = graph.get_point_count();

	m_positions.resize(point_count);
	m_thicknesses.resize(point_count);
	m_parent_indices.resize(point_count);

	for (int32_t i = 0; i < point_count; ++i) {
		const IvyPoint *point = graph.get_point(i);
		if (point == nullptr) {
			clear_results();
			return false;
		}
		m_positions.set(i, point->position);
		m_thicknesses.set(i, point->thickness);
		m_parent_indices.set(i, point->parent_index);
	}

	const int32_t target_count = m_solver.get_target_count();
	m_target_reached.resize(static_cast<size_t>(target_count));
	for (int32_t i = 0; i < target_count; ++i) {
		m_target_reached[static_cast<size_t>(i)] = m_solver.is_target_reached(i) ? 1 : 0;
	}

	return true;
}

int32_t IvyGenerator::get_point_count() const {
	return static_cast<int32_t>(m_positions.size());
}

PackedVector3Array IvyGenerator::get_point_positions() const {
	return m_positions;
}

PackedFloat32Array IvyGenerator::get_point_thicknesses() const {
	return m_thicknesses;
}

PackedInt32Array IvyGenerator::get_point_parent_indices() const {
	return m_parent_indices;
}

int32_t IvyGenerator::get_target_count() const {
	return static_cast<int32_t>(m_target_reached.size());
}

bool IvyGenerator::is_target_reached(int32_t target_index) const {
	if (target_index < 0 || target_index >= static_cast<int32_t>(m_target_reached.size())) {
		return false;
	}
	return m_target_reached[static_cast<size_t>(target_index)] != 0;
}

int32_t IvyGenerator::get_reached_target_count() const {
	int32_t count = 0;
	for (const uint8_t reached : m_target_reached) {
		if (reached != 0) {
			++count;
		}
	}
	return count;
}

} //namespace ivygenerator