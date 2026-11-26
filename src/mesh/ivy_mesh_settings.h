/*
 * Copyright (c) 2026 Max Heinze. All rights reserved.
 *
 * Licensed for non-commercial use with attribution only.
 * Commercial use is strictly prohibited.
 */

#pragma once

#include "ivy_mesh_config.h"

#include <godot_cpp/classes/resource.hpp>

#include <cstdint>

namespace ivygenerator {

class IvyMeshSettings : public godot::Resource {
	GDCLASS(IvyMeshSettings, godot::Resource)

protected:
	static void _bind_methods();

public:
	// Stem tube geometry
	void set_radial_segments(int32_t radial_segments);
	int32_t get_radial_segments() const;

	void set_radius_scale(float radius_scale);
	float get_radius_scale() const;

	void set_wall_offset_ratio(float wall_offset_ratio);
	float get_wall_offset_ratio() const;

	void set_uv_v_scale(float uv_v_scale);
	float get_uv_v_scale() const;

	void set_cap_tips(bool cap_tips);
	bool get_cap_tips() const;

	// Leaf placement
	void set_leaf_seed(int32_t leaf_seed);
	int32_t get_leaf_seed() const;

	void set_leaf_density(float leaf_density);
	float get_leaf_density() const;

	void set_leaf_min_thickness(float leaf_min_thickness);
	float get_leaf_min_thickness() const;

	void set_leaf_max_thickness(float leaf_max_thickness);
	float get_leaf_max_thickness() const;

	void set_leaf_size_min(float leaf_size_min);
	float get_leaf_size_min() const;

	void set_leaf_size_max(float leaf_size_max);
	float get_leaf_size_max() const;

	void set_leaf_tilt_degrees(float leaf_tilt_degrees);
	float get_leaf_tilt_degrees() const;

	void set_leaf_yaw_variation_degrees(float leaf_yaw_variation_degrees);
	float get_leaf_yaw_variation_degrees() const;

	void set_leaf_roll_variation_degrees(float leaf_roll_variation_degrees);
	float get_leaf_roll_variation_degrees() const;

	IvyMeshConfig to_mesh_config() const;

private:
	int32_t m_radial_segments = 6;
	float m_radius_scale = 0.5f;
	float m_wall_offset_ratio = 0.85f;
	float m_uv_v_scale = 1.0f;
	bool m_cap_tips = true;

	int32_t m_leaf_seed = 42;
	float m_leaf_density = 4.0f;
	float m_leaf_min_thickness = 0.0f;
	float m_leaf_max_thickness = 0.0f;
	float m_leaf_size_min = 0.08f;
	float m_leaf_size_max = 0.16f;
	float m_leaf_tilt_degrees = 30.0f;
	float m_leaf_yaw_variation_degrees = 30.0f;
	float m_leaf_roll_variation_degrees = 20.0f;
};

} // namespace ivygenerator