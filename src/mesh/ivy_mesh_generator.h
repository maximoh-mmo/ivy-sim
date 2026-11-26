/*
 * Copyright (c) 2026 Max Heinze. All rights reserved.
 *
 * Licensed for non-commercial use with attribution only.
 * Commercial use is strictly prohibited.
 */

#pragma once

#include "ivy_mesh_config.h"
#include "ivy_mesh_data.h"

#include <godot_cpp/variant/transform3d.hpp>
#include <godot_cpp/variant/vector3.hpp>

#include <cstdint>
#include <vector>

/**
 * @file ivy_mesh_generator.h
 * @brief Native C++ procedural 3D mesh generator for ivy growth graphs.
 */

namespace ivygenerator {

/**
 * @class IvyMeshGenerator
 * @brief Converts Layer 1 ivy growth graph arrays into smooth 3D tapered tube geometry.
 */
class IvyMeshGenerator {
public:
	/**
	 * @brief Builds a tapered cylindrical 3D stem mesh from topological graph arrays.
	 * @param positions World-space positions for each graph node.
	 * @param thicknesses Radial thickness values for each graph node (> 0).
	 * @param parent_indices Parent node indices (-1 for roots, 0 <= parent < index for children).
	 * @param wall_normal Surface normal of the wall plane (must be finite and non-zero).
	 * @param config Tube tessellation, wall-offset, and UV parameters.
	 * @param out_mesh Output mesh buffers (cleared on entry and on failure).
	 * @return bool True if inputs are valid and mesh buffers were built, false on validation failure.
	 */
	static bool build_stem_mesh(
			const std::vector<godot::Vector3> &positions,
			const std::vector<float> &thicknesses,
			const std::vector<int32_t> &parent_indices,
			const godot::Vector3 &wall_normal,
			const IvyMeshConfig &config,
			IvyMeshData &out_mesh);

	/**
	 * @brief Generates deterministic 3D leaf placement transforms along the vine graph.
	 * @details Local convention: origin at petiole attachment on the stem surface,
	 * -Z along the leaf blade tip, and +Y as the upper leaf face normal.
	 * @return bool True if inputs are valid (even if leaf_density == 0 produces empty output).
	 */
	static bool build_leaf_transforms(
			const std::vector<godot::Vector3> &positions,
			const std::vector<float> &thicknesses,
			const std::vector<int32_t> &parent_indices,
			const godot::Vector3 &wall_normal,
			const IvyMeshConfig &config,
			std::vector<godot::Transform3D> &out_transforms);
};

} // namespace ivygenerator