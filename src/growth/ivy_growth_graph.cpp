/*
 * Copyright (c) 2026 Max Heinze. All rights reserved.
 *
 * Licensed for non-commercial use with attribution only.
 * Commercial use is strictly prohibited.
 */

/**
 * @file ivy_growth_graph.cpp
 * @brief Implementation of IvyGrowthGraph node insertion, validation, and indexing.
 */

#include "ivy_growth_graph.h"

#include <limits>

namespace ivygenerator {

int32_t IvyGrowthGraph::add_point(const IvyPoint &point) {
	// Prevent indices from exceeding int32_t capacity.
	if (m_points.size() > static_cast<size_t>(
								  std::numeric_limits<int32_t>::max())) {
		return -1;
	}

	// Roots points have no parent, so we can just add them to the graph.
	if (point.parent_index != -1) {
		// If the parent index is invalid, we cannot add this point to the graph.
		if (point.parent_index < 0 || point.parent_index >= static_cast<int32_t>(m_points.size())) {
			return -1;
		}
	}

	// Add the point to the graph and return its index.
	const int32_t index = static_cast<int32_t>(m_points.size());
	m_points.push_back(point);
	return index;
}

const IvyPoint *IvyGrowthGraph::get_point(int32_t index) const {
	// Ensure the provided index is valid.
	if (index >= static_cast<int32_t>(m_points.size()) || index < 0) {
		return nullptr;
	}
	return &m_points[index];
}

int32_t IvyGrowthGraph::get_point_count() const {
	return static_cast<int32_t>(m_points.size());
}

void IvyGrowthGraph::clear() {
	m_points.clear();
}

} // namespace ivygenerator
