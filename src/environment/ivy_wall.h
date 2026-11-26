/*
 * Ivy Sim - Procedural ivy generator for Godot 4
 * Copyright (C) 2026 Max Heinze
 *
 * SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
 * Dual-licensed under GNU GPLv3 (see LICENSE) or a commercial license (see COMMERCIAL_LICENSE.md).
 * Distributed WITHOUT ANY WARRANTY; see LICENSE for details.
 */

#pragma once

#include <godot_cpp/variant/vector2.hpp>
#include <godot_cpp/variant/vector3.hpp>

/**
 * @file ivy_wall.h
 * @brief Planar surface representation for ivy growth adhesion.
 */

/**
 * @struct IvyWall
 * @brief Represents an oriented planar surface used as an adhesion target for ivy simulation.
 *
 * Defines a 3D rectangular plane defined by an origin point, two orthogonal tangent axes (U and V),
 * physical dimensions (width and height), and texture UV coordinate mapping parameters.
 */

namespace ivygenerator {

struct IvyWall {
	godot::Vector3 origin = godot::Vector3(); /**< World-space origin (bottom-left corner) of the wall. */
	godot::Vector3 axis_u = godot::Vector3(1.0f, 0.0f, 0.0f); /**< Unit tangent vector along the wall width (horizontal axis). */
	godot::Vector3 axis_v = godot::Vector3(0.0f, 1.0f, 0.0f); /**< Unit tangent vector along the wall height (vertical axis). */

	float width = 0.0f; /**< Physical width of the wall in world units. */
	float height = 0.0f; /**< Physical height of the wall in world units. */
	godot::Vector2 uv_scale = godot::Vector2(1.0f, 1.0f); /**< Texture UV tiling scale multiplier. */
	godot::Vector2 uv_offset = godot::Vector2(0.0f, 0.0f); /**< Texture UV coordinate offset. */

	/**
	 * @brief Projects a 3D world coordinate onto the local 2D plane coordinate system.
	 * @param world_position World-space position to project.
	 * @return Vector2 Projected local (U, V) coordinates relative to origin.
	 */
	godot::Vector2 world_to_local(const godot::Vector3 &world_position) const;

	/**
	 * @brief Transforms local 2D wall coordinates into a 3D world-space position.
	 * @param local_position Local (U, V) coordinate on the wall plane.
	 * @return Vector3 World-space position on the wall.
	 */
	godot::Vector3 local_to_world(const godot::Vector2 &local_position) const;

	/**
	 * @brief Converts local (U, V) wall coordinates to normalized texture UV coordinates.
	 * @param local_position Local (U, V) coordinate on the wall plane.
	 * @return Vector2 Scaled and offset UV coordinate.
	 */
	godot::Vector2 local_to_uv(const godot::Vector2 &local_position) const;

	/**
	 * @brief Calculates the unit normal vector perpendicular to the wall surface.
	 *
	 * Computes the cross product axis_u x axis_v. Returns a zero vector if the wall
	 * is degenerate.
	 *
	 * @return Vector3 Normalized surface normal vector.
	 */
	godot::Vector3 get_normal() const;

	/**
	 * @brief Checks if a local 2D coordinate falls strictly within the [0, width] x [0, height] bounds.
	 * @param local_position Local (U, V) coordinate on the wall plane.
	 * @return bool True if inside the rectangular boundaries.
	 */
	bool contains_local_point(const godot::Vector2 &local_position) const;

	/**
	 * @brief Determines whether a 3D world-space point lies on or near the wall surface.
	 *
	 * Verifies that perpendicular distance to the plane is within tolerance and that
	 * the projected local point lies within the wall rectangle expanded by tolerance.
	 *
	 * @param world_position World position to test.
	 * @param tolerance Distance threshold for plane proximity and boundary margin (default: 0.01f).
	 * @return bool True if the point lies within tolerance of the surface.
	 */
	bool is_on_surface(const godot::Vector3 &world_position, float tolerance = 0.01f) const;

	/**
	 * @brief Validates that dimensions, coordinates, and axes are finite, positive, normalized, and orthogonal.
	 * @return bool True if the wall configuration is geometrically valid.
	 */
	bool is_valid() const;
};

} // namespace ivygenerator
