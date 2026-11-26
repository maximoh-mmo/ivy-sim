/*
 * Ivy Sim - Procedural ivy generator for Godot 4
 * Copyright (C) 2026 Max Heinze
 *
 * SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
 * Dual-licensed under GNU GPLv3 (see LICENSE) or a commercial license (see COMMERCIAL_LICENSE.md).
 * Distributed WITHOUT ANY WARRANTY; see LICENSE for details.
 */

#include "ivy_mesh_settings.h"

#include <godot_cpp/core/class_db.hpp>

namespace ivygenerator {
using namespace godot;

void IvyMeshSettings::_bind_methods() {
	// Stem
	ClassDB::bind_method(D_METHOD("set_radial_segments", "radial_segments"), &IvyMeshSettings::set_radial_segments);
	ClassDB::bind_method(D_METHOD("get_radial_segments"), &IvyMeshSettings::get_radial_segments);
	ClassDB::bind_method(D_METHOD("set_radius_scale", "radius_scale"), &IvyMeshSettings::set_radius_scale);
	ClassDB::bind_method(D_METHOD("get_radius_scale"), &IvyMeshSettings::get_radius_scale);
	ClassDB::bind_method(D_METHOD("set_wall_offset_ratio", "wall_offset_ratio"), &IvyMeshSettings::set_wall_offset_ratio);
	ClassDB::bind_method(D_METHOD("get_wall_offset_ratio"), &IvyMeshSettings::get_wall_offset_ratio);
	ClassDB::bind_method(D_METHOD("set_uv_v_scale", "uv_v_scale"), &IvyMeshSettings::set_uv_v_scale);
	ClassDB::bind_method(D_METHOD("get_uv_v_scale"), &IvyMeshSettings::get_uv_v_scale);
	ClassDB::bind_method(D_METHOD("set_cap_tips", "cap_tips"), &IvyMeshSettings::set_cap_tips);
	ClassDB::bind_method(D_METHOD("get_cap_tips"), &IvyMeshSettings::get_cap_tips);

	ADD_GROUP("Stem", "");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "radial_segments", PROPERTY_HINT_RANGE, "3,32,1,or_greater"), "set_radial_segments", "get_radial_segments");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "radius_scale", PROPERTY_HINT_RANGE, "0.01,5.0,0.01,or_greater"), "set_radius_scale", "get_radius_scale");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "wall_offset_ratio", PROPERTY_HINT_RANGE, "0.0,2.0,0.01,or_greater"), "set_wall_offset_ratio", "get_wall_offset_ratio");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "uv_v_scale", PROPERTY_HINT_RANGE, "0.01,10.0,0.01,or_greater"), "set_uv_v_scale", "get_uv_v_scale");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "cap_tips"), "set_cap_tips", "get_cap_tips");

	// Leaves
	ClassDB::bind_method(D_METHOD("set_leaf_seed", "leaf_seed"), &IvyMeshSettings::set_leaf_seed);
	ClassDB::bind_method(D_METHOD("get_leaf_seed"), &IvyMeshSettings::get_leaf_seed);
	ClassDB::bind_method(D_METHOD("set_leaf_density", "leaf_density"), &IvyMeshSettings::set_leaf_density);
	ClassDB::bind_method(D_METHOD("get_leaf_density"), &IvyMeshSettings::get_leaf_density);
	ClassDB::bind_method(D_METHOD("set_leaf_min_thickness", "leaf_min_thickness"), &IvyMeshSettings::set_leaf_min_thickness);
	ClassDB::bind_method(D_METHOD("get_leaf_min_thickness"), &IvyMeshSettings::get_leaf_min_thickness);
	ClassDB::bind_method(D_METHOD("set_leaf_max_thickness", "leaf_max_thickness"), &IvyMeshSettings::set_leaf_max_thickness);
	ClassDB::bind_method(D_METHOD("get_leaf_max_thickness"), &IvyMeshSettings::get_leaf_max_thickness);
	ClassDB::bind_method(D_METHOD("set_leaf_size_min", "leaf_size_min"), &IvyMeshSettings::set_leaf_size_min);
	ClassDB::bind_method(D_METHOD("get_leaf_size_min"), &IvyMeshSettings::get_leaf_size_min);
	ClassDB::bind_method(D_METHOD("set_leaf_size_max", "leaf_size_max"), &IvyMeshSettings::set_leaf_size_max);
	ClassDB::bind_method(D_METHOD("get_leaf_size_max"), &IvyMeshSettings::get_leaf_size_max);
	ClassDB::bind_method(D_METHOD("set_leaf_tilt_degrees", "leaf_tilt_degrees"), &IvyMeshSettings::set_leaf_tilt_degrees);
	ClassDB::bind_method(D_METHOD("get_leaf_tilt_degrees"), &IvyMeshSettings::get_leaf_tilt_degrees);
	ClassDB::bind_method(D_METHOD("set_leaf_yaw_variation_degrees", "leaf_yaw_variation_degrees"), &IvyMeshSettings::set_leaf_yaw_variation_degrees);
	ClassDB::bind_method(D_METHOD("get_leaf_yaw_variation_degrees"), &IvyMeshSettings::get_leaf_yaw_variation_degrees);
	ClassDB::bind_method(D_METHOD("set_leaf_roll_variation_degrees", "leaf_roll_variation_degrees"), &IvyMeshSettings::set_leaf_roll_variation_degrees);
	ClassDB::bind_method(D_METHOD("get_leaf_roll_variation_degrees"), &IvyMeshSettings::get_leaf_roll_variation_degrees);

	ADD_GROUP("Leaves", "");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "leaf_seed"), "set_leaf_seed", "get_leaf_seed");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "leaf_density", PROPERTY_HINT_RANGE, "0.0,30.0,0.1,or_greater"), "set_leaf_density", "get_leaf_density");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "leaf_min_thickness", PROPERTY_HINT_RANGE, "0.0,2.0,0.005,or_greater"), "set_leaf_min_thickness", "get_leaf_min_thickness");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "leaf_max_thickness", PROPERTY_HINT_RANGE, "0.0,2.0,0.005,or_greater"), "set_leaf_max_thickness", "get_leaf_max_thickness");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "leaf_size_min", PROPERTY_HINT_RANGE, "0.005,1.0,0.005,or_greater"), "set_leaf_size_min", "get_leaf_size_min");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "leaf_size_max", PROPERTY_HINT_RANGE, "0.005,1.0,0.005,or_greater"), "set_leaf_size_max", "get_leaf_size_max");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "leaf_tilt_degrees", PROPERTY_HINT_RANGE, "0.0,90.0,0.5"), "set_leaf_tilt_degrees", "get_leaf_tilt_degrees");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "leaf_yaw_variation_degrees", PROPERTY_HINT_RANGE, "0.0,180.0,0.5"), "set_leaf_yaw_variation_degrees", "get_leaf_yaw_variation_degrees");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "leaf_roll_variation_degrees", PROPERTY_HINT_RANGE, "0.0,90.0,0.5"), "set_leaf_roll_variation_degrees", "get_leaf_roll_variation_degrees");
}

void IvyMeshSettings::set_radial_segments(int32_t radial_segments) { m_radial_segments = radial_segments; }
int32_t IvyMeshSettings::get_radial_segments() const { return m_radial_segments; }

void IvyMeshSettings::set_radius_scale(float radius_scale) { m_radius_scale = radius_scale; }
float IvyMeshSettings::get_radius_scale() const { return m_radius_scale; }

void IvyMeshSettings::set_wall_offset_ratio(float wall_offset_ratio) { m_wall_offset_ratio = wall_offset_ratio; }
float IvyMeshSettings::get_wall_offset_ratio() const { return m_wall_offset_ratio; }

void IvyMeshSettings::set_uv_v_scale(float uv_v_scale) { m_uv_v_scale = uv_v_scale; }
float IvyMeshSettings::get_uv_v_scale() const { return m_uv_v_scale; }

void IvyMeshSettings::set_cap_tips(bool cap_tips) { m_cap_tips = cap_tips; }
bool IvyMeshSettings::get_cap_tips() const { return m_cap_tips; }

void IvyMeshSettings::set_leaf_seed(int32_t leaf_seed) { m_leaf_seed = leaf_seed; }
int32_t IvyMeshSettings::get_leaf_seed() const { return m_leaf_seed; }

void IvyMeshSettings::set_leaf_density(float leaf_density) { m_leaf_density = leaf_density; }
float IvyMeshSettings::get_leaf_density() const { return m_leaf_density; }

void IvyMeshSettings::set_leaf_min_thickness(float leaf_min_thickness) { m_leaf_min_thickness = leaf_min_thickness; }
float IvyMeshSettings::get_leaf_min_thickness() const { return m_leaf_min_thickness; }

void IvyMeshSettings::set_leaf_max_thickness(float leaf_max_thickness) { m_leaf_max_thickness = leaf_max_thickness; }
float IvyMeshSettings::get_leaf_max_thickness() const { return m_leaf_max_thickness; }

void IvyMeshSettings::set_leaf_size_min(float leaf_size_min) { m_leaf_size_min = leaf_size_min; }
float IvyMeshSettings::get_leaf_size_min() const { return m_leaf_size_min; }

void IvyMeshSettings::set_leaf_size_max(float leaf_size_max) { m_leaf_size_max = leaf_size_max; }
float IvyMeshSettings::get_leaf_size_max() const { return m_leaf_size_max; }

void IvyMeshSettings::set_leaf_tilt_degrees(float leaf_tilt_degrees) { m_leaf_tilt_degrees = leaf_tilt_degrees; }
float IvyMeshSettings::get_leaf_tilt_degrees() const { return m_leaf_tilt_degrees; }

void IvyMeshSettings::set_leaf_yaw_variation_degrees(float leaf_yaw_variation_degrees) { m_leaf_yaw_variation_degrees = leaf_yaw_variation_degrees; }
float IvyMeshSettings::get_leaf_yaw_variation_degrees() const { return m_leaf_yaw_variation_degrees; }

void IvyMeshSettings::set_leaf_roll_variation_degrees(float leaf_roll_variation_degrees) { m_leaf_roll_variation_degrees = leaf_roll_variation_degrees; }
float IvyMeshSettings::get_leaf_roll_variation_degrees() const { return m_leaf_roll_variation_degrees; }

IvyMeshConfig IvyMeshSettings::to_mesh_config() const {
	constexpr float deg_to_rad = 3.14159265358979323846f / 180.0f;

	IvyMeshConfig config;
	config.radial_segments = m_radial_segments;
	config.radius_scale = m_radius_scale;
	config.wall_offset_ratio = m_wall_offset_ratio;
	config.uv_v_scale = m_uv_v_scale;
	config.cap_tips = m_cap_tips;

	config.leaf_seed = m_leaf_seed;
	config.leaf_density = m_leaf_density;
	config.leaf_min_thickness = m_leaf_min_thickness;
	config.leaf_max_thickness = m_leaf_max_thickness;
	config.leaf_size_min = m_leaf_size_min;
	config.leaf_size_max = m_leaf_size_max;
	config.leaf_tilt_radians = m_leaf_tilt_degrees * deg_to_rad;
	config.leaf_yaw_variation = m_leaf_yaw_variation_degrees * deg_to_rad;
	config.leaf_roll_variation = m_leaf_roll_variation_degrees * deg_to_rad;

	return config;
}

} // namespace ivygenerator