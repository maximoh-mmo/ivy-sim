/*
 * Copyright (c) 2026 Max Heinze. All rights reserved.
 *
 * Licensed for non-commercial use with attribution only.
 * Commercial use is strictly prohibited.
 */

#pragma once

#include <godot_cpp/variant/vector3.hpp>

#include <cstdint>

/**
 * @file ivy_point.h
 * @brief Representation of an individual ivy node/particle in the simulation graph.
 */

namespace ivygenerator {

/**
 * @struct IvyPoint
 * @brief Represents a single point/node along an ivy vine within the growth graph.
 */
struct IvyPoint {
	godot::Vector3 position = godot::Vector3(); /**< 3D world-space position of the ivy point. */
	float thickness = 0.0f; /**< Vine radius/thickness at this point. */
	int32_t parent_index = -1; /**< Index of the parent point in the graph (-1 for root nodes). */
};

} // namespace ivygenerator
