/*
 * Ivy Sim - Procedural ivy generator for Godot 4
 * Copyright (C) 2026 Max Heinze
 *
 * SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
 * Dual-licensed under GNU GPLv3 (see LICENSE) or a commercial license (see COMMERCIAL_LICENSE.md).
 * Distributed WITHOUT ANY WARRANTY; see LICENSE for details.
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
