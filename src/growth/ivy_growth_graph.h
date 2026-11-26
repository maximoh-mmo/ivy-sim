/*
 * Ivy Sim - Procedural ivy generator for Godot 4
 * Copyright (C) 2026 Max Heinze
 *
 * SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
 * Dual-licensed under GNU GPLv3 (see LICENSE) or a commercial license (see COMMERCIAL_LICENSE.md).
 * Distributed WITHOUT ANY WARRANTY; see LICENSE for details.
 */

#pragma once

#include "ivy_point.h"

#include <cstdint>
#include <vector>

/**
 * @file ivy_growth_graph.h
 * @brief Graph data structure managing ivy simulation points, branches, and parent-child hierarchies.
 */

namespace ivygenerator {

/**
 * @class IvyGrowthGraph
 * @brief Directed tree/graph structure storing and indexing all nodes of an ivy simulation.
 *
 * Validates point connectivity, supporting root insertion, branching, retrieval by index,
 * and graph resets.
 */
class IvyGrowthGraph {
private:
	std::vector<IvyPoint> m_points; /**< Sequential storage of all ivy points in the graph. */

public:
	/**
	 * @brief Adds a new point to the graph after validating its parent connectivity.
	 *
	 * Roots must specify a parent_index of -1. Child points must specify an existing,
	 * valid parent index in [0, point_count - 1].
	 *
	 * @param point The IvyPoint to add.
	 * @return int32_t Assigned index of the newly added point, or -1 if parent validation failed or capacity exceeded.
	 */
	int32_t add_point(const IvyPoint &point);

	/**
	 * @brief Retrieves a pointer to the IvyPoint at the specified index.
	 * @param index Zero-based point index.
	 * @return const IvyPoint* Pointer to the point if index is valid, nullptr otherwise.
	 */
	const IvyPoint *get_point(int32_t index) const;

	/**
	 * @brief Returns the total number of points currently stored in the graph.
	 * @return int32_t Number of points.
	 */
	int32_t get_point_count() const;

	/**
	 * @brief Updates the thickness of an existing point in the graph.
	 * @param index Zero-based point index.
	 * @param thickness New finite, non-negative thickness value.
	 * @return bool True if the index and thickness were valid and the point was updated.
	 */
	bool set_point_thickness(int32_t index, float thickness);

	/**
	 * @brief Clears all points from the graph.
	 */
	void clear();
};

} // namespace ivygenerator
