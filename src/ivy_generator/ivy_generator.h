/*
 * Copyright (c) 2026 Max Heinze. All rights reserved.
 *
 * Licensed for non-commercial use with attribution only.
 * Commercial use is strictly prohibited.
 */

#pragma once

#include "environment/ivy_surface_sampler.h"
#include "environment/ivy_wall.h"
#include "growth/ivy_growth_solver.h"
#include "random/ivy_random.h"
#include "settings/ivy_growth_config.h"
#include "settings/ivy_settings.h"

#include <cstdint>
#include <godot_cpp/classes/image.hpp>
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/variant/packed_float32_array.hpp>
#include <godot_cpp/variant/packed_int32_array.hpp>
#include <godot_cpp/variant/packed_vector3_array.hpp>
#include <godot_cpp/variant/vector2.hpp>
#include <godot_cpp/variant/vector3.hpp>
#include <vector>

/**
 * @file ivy_generator.h
 * @brief Core entry point class for the Ivy simulation GDExtension.
 */

/**
 * @class IvyGenerator
 * @brief Godot RefCounted node providing high-level ivy simulation generation and configuration.
 *
 * Exposes methods to GDScript for initiating growth simulation runs, managing random seeds,
 * and coordinating simulation subsystems.
 */

namespace ivygenerator {

class IvyGenerator : public godot::RefCounted {
	GDCLASS(IvyGenerator, godot::RefCounted)

protected:
	/**
	 * @brief Godot ClassDB method registration hook.
	 */
	static void _bind_methods();

public:
	IvyGenerator() = default;
	~IvyGenerator() override = default;

	/**
	 * @brief Assigns the IvySettings resource containing growth parameters.
	 * @param settings Reference to configured IvySettings resource.
	 */
	void set_settings(const godot::Ref<IvySettings> &settings);

	/**
	 * @brief Retrieves the active IvySettings resource.
	 * @return Reference to current IvySettings.
	 */
	godot::Ref<IvySettings> get_settings() const;

	/**
	 * @brief Configures the planar wall surface for ivy growth.
	 * @param origin 3D world origin of the bottom-left wall corner.
	 * @param axis_u Unit vector defining the horizontal wall axis (+U).
	 * @param axis_v Unit vector defining the vertical wall axis (+V).
	 * @param width Physical width of the wall along axis_u.
	 * @param height Physical height of the wall along axis_v.
	 * @param uv_scale UV texture scaling multiplier.
	 * @param uv_offset UV texture offset coordinate.
	 * @return True if wall configuration parameters are valid.
	 */
	bool set_wall(
			const godot::Vector3 &origin,
			const godot::Vector3 &axis_u,
			const godot::Vector3 &axis_v,
			float width,
			float height,
			const godot::Vector2 &uv_scale = godot::Vector2(1.0f, 1.0f),
			const godot::Vector2 &uv_offset = godot::Vector2(0.0f, 0.0f));

	/**
	 * @brief Sets the attraction/obstacle scalar guidance image.
	 * @param image Godot Image containing grayscale attraction values.
	 * @return True if image data was valid and loaded successfully.
	 */
	bool set_attraction_image(const godot::Ref<godot::Image> &image);

	/**
	 * @brief Defines explicit starting points on the wall plane.
	 * @param start_points Array of 3D positions on the wall surface.
	 */
	void set_start_points(const godot::PackedVector3Array &start_points);

	/**
	 * @brief Retrieves the configured explicit start points.
	 * @return Array of explicit 3D starting positions.
	 */
	godot::PackedVector3Array get_start_points() const;

	/**
	 * @brief Clears all explicit starting positions.
	 */
	void clear_start_points();

	/**
	 * @brief Defines optional target waypoints on the wall plane to steer nearby growth.
	 * @param target_points Array of 3D target positions on the wall surface.
	 */
	void set_target_points(const godot::PackedVector3Array &target_points);

	/**
	 * @brief Retrieves the configured target waypoints.
	 * @return Array of configured 3D target positions.
	 */
	godot::PackedVector3Array get_target_points() const;

	/**
	 * @brief Clears all configured target waypoints.
	 */
	void clear_target_points();

	/**
	 * @brief Executes procedural ivy simulation and populates graph buffers.
	 * @return True on successful simulation completion, false on error.
	 */
	bool generate();

	/**
	 * @brief Retrieves the total point count in the generated graph.
	 * @return Total number of generated points.
	 */
	int32_t get_point_count() const;

	/**
	 * @brief Retrieves all point world positions.
	 * @return Packed array of 3D world coordinates.
	 */
	godot::PackedVector3Array get_point_positions() const;

	/**
	 * @brief Retrieves all point radial thicknesses.
	 * @return Packed array of float thickness values.
	 */
	godot::PackedFloat32Array get_point_thicknesses() const;

	/**
	 * @brief Retrieves all point parent topological indices.
	 * @return Packed array of parent indices (-1 for root nodes).
	 */
	godot::PackedInt32Array get_point_parent_indices() const;

	/**
	 * @brief Returns the number of target waypoints evaluated in the last generate() run.
	 * @return Target count from the last successful generate(), or 0 if none/failed.
	 */
	int32_t get_target_count() const;

	/**
	 * @brief Checks whether a target waypoint was satisfied within acceptance radius in the last generate() run.
	 * @param target_index Zero-based target index.
	 * @return True if valid index and target was reached.
	 */
	bool is_target_reached(int32_t target_index) const;

	/**
	 * @brief Returns the total number of target waypoints reached in the last generate() run.
	 * @return Count of satisfied targets.
	 */
	int32_t get_reached_target_count() const;

private:
	void clear_results();
	godot::Ref<IvySettings> m_settings;
	IvyWall m_wall;
	IvySurfaceSampler m_surface_sampler;
	IvyGrowthConfig m_growth_config;
	IvyRandom m_random;
	IvyGrowthSolver m_solver;
	godot::PackedVector3Array m_explicit_start_points;
	godot::PackedVector3Array m_target_points;
	godot::PackedVector3Array m_positions;
	godot::PackedFloat32Array m_thicknesses;
	godot::PackedInt32Array m_parent_indices;
	std::vector<uint8_t> m_target_reached;
};
} // namespace ivygenerator