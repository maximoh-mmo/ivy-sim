/*
 * Ivy Sim - Procedural ivy generator for Godot 4
 * Copyright (C) 2026 Max Heinze
 *
 * SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
 * Dual-licensed under GNU GPLv3 (see LICENSE) or a commercial license (see COMMERCIAL_LICENSE.md).
 * Distributed WITHOUT ANY WARRANTY; see LICENSE for details.
 */

/**
 * @file ivy_mesh_generator.cpp
 * @brief Implementation of native C++ 3D ivy stem tube generation.
 */

#include "ivy_mesh_generator.h"
#include "../random/ivy_random.h"

#include <godot_cpp/variant/basis.hpp>

#include <algorithm>
#include <cmath>

namespace ivygenerator {
using namespace godot;

namespace {

constexpr float k_two_pi = 6.28318530717958647692f;
constexpr float k_epsilon_sq = 1e-10f;

struct StemChain {
	std::vector<int32_t> node_indices;
	int32_t branch_order = 0;
	bool starts_at_fork = false;
};

struct PendingChainStart {
	int32_t start_node = -1;
	int32_t first_child = -1; // -1 for root chains; >= 0 for side branches starting at fork
	int32_t branch_order = 0;
};

bool validate_config(const IvyMeshConfig &config) {
	if (config.radial_segments < 3) {
		return false;
	}
	if (!std::isfinite(config.radius_scale) || config.radius_scale <= 0.0f) {
		return false;
	}
	if (!std::isfinite(config.wall_offset_ratio) || config.wall_offset_ratio < 0.0f) {
		return false;
	}
	if (!std::isfinite(config.uv_v_scale) || config.uv_v_scale <= 0.0f) {
		return false;
	}
	return true;
}

Vector3 compute_fallback_perpendicular(const Vector3 &unit_vec) {
	const Vector3 ref = (std::abs(unit_vec.y) < 0.9f)
			? Vector3(0.0f, 1.0f, 0.0f)
			: Vector3(1.0f, 0.0f, 0.0f);
	return ref.cross(unit_vec).normalized();
}

std::vector<StemChain> decompose_chains(
		const std::vector<float> &thicknesses,
		const std::vector<int32_t> &parent_indices) {
	const int32_t count = static_cast<int32_t>(parent_indices.size());
	std::vector<std::vector<int32_t>> children(static_cast<size_t>(count));
	std::vector<PendingChainStart> pending;

	for (int32_t i = 0; i < count; ++i) {
		const int32_t parent = parent_indices[static_cast<size_t>(i)];
		if (parent == -1) {
			pending.push_back({ i, -1, 0 });
		} else {
			children[static_cast<size_t>(parent)].push_back(i);
		}
	}

	std::vector<StemChain> chains;
	size_t pending_idx = 0;

	while (pending_idx < pending.size()) {
		const PendingChainStart start = pending[pending_idx++];

		StemChain chain;
		chain.branch_order = start.branch_order;
		chain.starts_at_fork = (start.first_child >= 0);

		int32_t current = start.start_node;
		chain.node_indices.push_back(current);

		if (start.first_child >= 0) {
			current = start.first_child;
			chain.node_indices.push_back(current);
		}

		while (!children[static_cast<size_t>(current)].empty()) {
			const std::vector<int32_t> &node_children = children[static_cast<size_t>(current)];

			// Continue the current tube along the thickest child (tie-break: smallest index).
			int32_t primary_child = node_children[0];
			float primary_thickness = thicknesses[static_cast<size_t>(primary_child)];
			for (size_t c = 1; c < node_children.size(); ++c) {
				const int32_t candidate = node_children[c];
				const float candidate_thickness = thicknesses[static_cast<size_t>(candidate)];
				if (candidate_thickness > primary_thickness) {
					primary_child = candidate;
					primary_thickness = candidate_thickness;
				}
			}

			// Secondary children start fresh chains seated at the fork parent.
			for (const int32_t child : node_children) {
				if (child != primary_child) {
					pending.push_back({ current, child, start.branch_order + 1 });
				}
			}

			current = primary_child;
			chain.node_indices.push_back(current);
		}

		if (chain.node_indices.size() >= 2) {
			chains.push_back(std::move(chain));
		}
	}

	return chains;
}

void append_chain_tube(
		const StemChain &chain,
		const std::vector<Vector3> &positions,
		const std::vector<float> &thicknesses,
		float max_thickness,
		const Vector3 &normal,
		const IvyMeshConfig &config,
		IvyMeshData &out_mesh) {
	const size_t ring_count = chain.node_indices.size();
	const int32_t segs = config.radial_segments;
	const int32_t stride = segs + 1;

	// 1. Compute ring radii, wall-offset centers, and effective thicknesses.
	std::vector<float> radii(ring_count);
	std::vector<float> effective_thicknesses(ring_count);
	std::vector<Vector3> centers(ring_count);

	for (size_t k = 0; k < ring_count; ++k) {
		const size_t idx = static_cast<size_t>(chain.node_indices[k]);
		if (k == 0 && chain.starts_at_fork) {
			const size_t child_idx = static_cast<size_t>(chain.node_indices[1]);
			const float parent_radius = thicknesses[idx] * config.radius_scale;
			const float child_radius = thicknesses[child_idx] * config.radius_scale;
			effective_thicknesses[0] = std::min(thicknesses[idx], thicknesses[child_idx]);
			radii[0] = std::min(parent_radius, child_radius);
			// Seat ring 0 inside the parent's offset centerline to avoid gaps at forks.
			centers[0] = positions[idx] + normal * (parent_radius * config.wall_offset_ratio);
		} else {
			effective_thicknesses[k] = thicknesses[idx];
			radii[k] = thicknesses[idx] * config.radius_scale;
			centers[k] = positions[idx] + normal * (radii[k] * config.wall_offset_ratio);
		}
	}

	// 2. Compute smooth tangents and cumulative arc lengths along the chain.
	std::vector<Vector3> tangents(ring_count);
	std::vector<float> arc_lengths(ring_count, 0.0f);
	Vector3 last_valid_dir = compute_fallback_perpendicular(normal);

	for (size_t k = 1; k < ring_count; ++k) {
		const Vector3 seg = centers[k] - centers[k - 1];
		arc_lengths[k] = arc_lengths[k - 1] + static_cast<float>(seg.length());
		if (seg.length_squared() > k_epsilon_sq) {
			last_valid_dir = seg.normalized();
		}
	}

	// Re-seed fallback from the first non-degenerate segment if available.
	for (size_t k = 1; k < ring_count; ++k) {
		const Vector3 seg = centers[k] - centers[k - 1];
		if (seg.length_squared() > k_epsilon_sq) {
			last_valid_dir = seg.normalized();
			break;
		}
	}

	for (size_t k = 0; k < ring_count; ++k) {
		Vector3 dir;
		if (k == 0) {
			const Vector3 fwd = centers[1] - centers[0];
			dir = (fwd.length_squared() > k_epsilon_sq) ? fwd.normalized() : last_valid_dir;
		} else if (k == ring_count - 1) {
			const Vector3 bwd = centers[k] - centers[k - 1];
			dir = (bwd.length_squared() > k_epsilon_sq) ? bwd.normalized() : last_valid_dir;
		} else {
			const Vector3 bwd = centers[k] - centers[k - 1];
			const Vector3 fwd = centers[k + 1] - centers[k];
			Vector3 blended;
			if (bwd.length_squared() > k_epsilon_sq) {
				blended += bwd.normalized();
			}
			if (fwd.length_squared() > k_epsilon_sq) {
				blended += fwd.normalized();
			}
			if (blended.length_squared() > k_epsilon_sq) {
				dir = blended.normalized();
			} else if (fwd.length_squared() > k_epsilon_sq) {
				dir = fwd.normalized();
			} else {
				dir = last_valid_dir;
			}
		}
		tangents[k] = dir;
		last_valid_dir = dir;
	}

	// 3. Emit ring vertices (radial_segments + 1 per ring for seam-free UVs).
	const int32_t base_vertex = static_cast<int32_t>(out_mesh.vertices.size());
	const float total_length = arc_lengths[ring_count - 1];
	const float branch_color_b = std::clamp(static_cast<float>(chain.branch_order) / 4.0f, 0.0f, 1.0f);

	for (size_t k = 0; k < ring_count; ++k) {
		const Vector3 &tangent = tangents[k];
		Vector3 out = normal - tangent * tangent.dot(normal);
		if (out.length_squared() > k_epsilon_sq) {
			out = out.normalized();
		} else {
			out = compute_fallback_perpendicular(tangent);
		}
		const Vector3 right = tangent.cross(out).normalized();

		const float radius = radii[k];
		const float uv_v = arc_lengths[k] * config.uv_v_scale;
		const float chain_t = (total_length > 1e-6f)
				? (arc_lengths[k] / total_length)
				: (static_cast<float>(k) / static_cast<float>(ring_count - 1));
		const float thickness_color_r = std::clamp(effective_thicknesses[k] / max_thickness, 0.0f, 1.0f);
		const Color ring_color(thickness_color_r, chain_t, branch_color_b, 1.0f);

		for (int32_t s = 0; s <= segs; ++s) {
			const float u = static_cast<float>(s) / static_cast<float>(segs);
			const float angle = k_two_pi * u;
			const Vector3 radial_dir = (out * std::cos(angle) + right * std::sin(angle)).normalized();

			out_mesh.vertices.push_back(centers[k] + radial_dir * radius);
			out_mesh.normals.push_back(radial_dir);
			out_mesh.uvs.push_back(Vector2(u, uv_v));
			out_mesh.colors.push_back(ring_color);
		}
	}

	// 4. Emit tube body triangles (Godot CW front-face winding).
	for (size_t k = 0; k + 1 < ring_count; ++k) {
		const int32_t r0 = base_vertex + static_cast<int32_t>(k) * stride;
		const int32_t r1 = base_vertex + static_cast<int32_t>(k + 1) * stride;

		for (int32_t s = 0; s < segs; ++s) {
			const int32_t a = r0 + s;
			const int32_t b = r0 + s + 1;
			const int32_t c = r1 + s + 1;
			const int32_t d = r1 + s;

			out_mesh.indices.push_back(a);
			out_mesh.indices.push_back(d);
			out_mesh.indices.push_back(c);

			out_mesh.indices.push_back(a);
			out_mesh.indices.push_back(c);
			out_mesh.indices.push_back(b);
		}
	}

	// 5. Optional tapered tip cap at the terminal ring (Godot CW front-face winding).
	if (config.cap_tips) {
		const size_t last = ring_count - 1;
		const float tip_ext = radii[last] * 0.5f;
		const int32_t last_ring_start = base_vertex + static_cast<int32_t>(last) * stride;
		const int32_t apex_index = static_cast<int32_t>(out_mesh.vertices.size());

		const float tip_color_r = std::clamp(effective_thicknesses[last] / max_thickness, 0.0f, 1.0f);
		out_mesh.vertices.push_back(centers[last] + tangents[last] * tip_ext);
		out_mesh.normals.push_back(tangents[last]);
		out_mesh.uvs.push_back(Vector2(0.5f, (total_length + tip_ext) * config.uv_v_scale));
		out_mesh.colors.push_back(Color(tip_color_r, 1.0f, branch_color_b, 1.0f));

		for (int32_t s = 0; s < segs; ++s) {
			out_mesh.indices.push_back(last_ring_start + s);
			out_mesh.indices.push_back(apex_index);
			out_mesh.indices.push_back(last_ring_start + s + 1);
		}
	}
}

bool validate_leaf_config(const IvyMeshConfig &config) {
	if (!std::isfinite(config.radius_scale) || config.radius_scale <= 0.0f) {
		return false;
	}
	if (!std::isfinite(config.wall_offset_ratio) || config.wall_offset_ratio < 0.0f) {
		return false;
	}
	if (!std::isfinite(config.leaf_density) || config.leaf_density < 0.0f) {
		return false;
	}
	if (!std::isfinite(config.leaf_min_thickness) || config.leaf_min_thickness < 0.0f) {
		return false;
	}
	if (!std::isfinite(config.leaf_max_thickness) || config.leaf_max_thickness < 0.0f) {
		return false;
	}
	if (config.leaf_max_thickness > 0.0f && config.leaf_max_thickness < config.leaf_min_thickness) {
		return false;
	}
	if (!std::isfinite(config.leaf_size_min) || config.leaf_size_min <= 0.0f ||
			!std::isfinite(config.leaf_size_max) || config.leaf_size_max < config.leaf_size_min) {
		return false;
	}
	constexpr float half_pi_upper = 1.5707964f;
	if (!std::isfinite(config.leaf_tilt_radians) ||
			config.leaf_tilt_radians < 0.0f || config.leaf_tilt_radians > half_pi_upper) {
		return false;
	}
	if (!std::isfinite(config.leaf_yaw_variation) || config.leaf_yaw_variation < 0.0f ||
			!std::isfinite(config.leaf_roll_variation) || config.leaf_roll_variation < 0.0f) {
		return false;
	}
	return true;
}

bool validate_graph_inputs(
		const std::vector<Vector3> &positions,
		const std::vector<float> &thicknesses,
		const std::vector<int32_t> &parent_indices,
		const Vector3 &wall_normal,
		float &out_max_thickness) {
	if (!wall_normal.is_finite() || wall_normal.length_squared() <= 1e-6f) {
		return false;
	}
	if (positions.empty() ||
			positions.size() != thicknesses.size() ||
			positions.size() != parent_indices.size()) {
		return false;
	}

	out_max_thickness = 0.0f;
	const int32_t count = static_cast<int32_t>(positions.size());
	for (int32_t i = 0; i < count; ++i) {
		const size_t idx = static_cast<size_t>(i);
		if (!positions[idx].is_finite() ||
				!std::isfinite(thicknesses[idx]) || thicknesses[idx] <= 0.0f) {
			return false;
		}
		const int32_t parent = parent_indices[idx];
		if (parent < -1 || parent >= i) {
			return false;
		}
		out_max_thickness = std::max(out_max_thickness, thicknesses[idx]);
	}
	return true;
}

void append_chain_leaves(
		const StemChain &chain,
		const std::vector<Vector3> &positions,
		const std::vector<float> &thicknesses,
		const Vector3 &normal,
		const IvyMeshConfig &config,
		IvyRandom &rng,
		std::vector<Transform3D> &out_transforms) {
	const size_t ring_count = chain.node_indices.size();
	std::vector<float> radii(ring_count);
	std::vector<float> effective_thicknesses(ring_count);
	std::vector<Vector3> centers(ring_count);

	for (size_t k = 0; k < ring_count; ++k) {
		const size_t idx = static_cast<size_t>(chain.node_indices[k]);
		if (k == 0 && chain.starts_at_fork) {
			const size_t child_idx = static_cast<size_t>(chain.node_indices[1]);
			const float parent_radius = thicknesses[idx] * config.radius_scale;
			const float child_radius = thicknesses[child_idx] * config.radius_scale;
			effective_thicknesses[0] = std::min(thicknesses[idx], thicknesses[child_idx]);
			radii[0] = std::min(parent_radius, child_radius);
			centers[0] = positions[idx] + normal * (parent_radius * config.wall_offset_ratio);
		} else {
			effective_thicknesses[k] = thicknesses[idx];
			radii[k] = thicknesses[idx] * config.radius_scale;
			centers[k] = positions[idx] + normal * (radii[k] * config.wall_offset_ratio);
		}
	}

	const float spacing = 1.0f / config.leaf_density;
	float next_leaf_dist = rng.random_float(0.15f * spacing, 0.85f * spacing);
	float side_sign = rng.random_bool() ? 1.0f : -1.0f;
	float arc = 0.0f;

	const float cos_tilt = std::cos(config.leaf_tilt_radians);
	const float sin_tilt = std::sin(config.leaf_tilt_radians);

	for (size_t k = 1; k < ring_count; ++k) {
		const Vector3 seg = centers[k] - centers[k - 1];
		const float seg_len = static_cast<float>(seg.length());
		if (seg_len <= 1e-6f) {
			continue;
		}

		const Vector3 tangent = seg / seg_len;
		Vector3 out = normal - tangent * tangent.dot(normal);
		out = (out.length_squared() > k_epsilon_sq)
				? out.normalized()
				: compute_fallback_perpendicular(tangent);
		const Vector3 right = tangent.cross(out).normalized();

		const float seg_end = arc + seg_len;
		while (next_leaf_dist <= seg_end) {
			const float t = std::clamp((next_leaf_dist - arc) / seg_len, 0.0f, 1.0f);
			const float thickness = effective_thicknesses[k - 1] +
					(effective_thicknesses[k] - effective_thicknesses[k - 1]) * t;

			const bool passes_min = thickness >= config.leaf_min_thickness;
			const bool passes_max =
					(config.leaf_max_thickness <= 0.0f) || (thickness <= config.leaf_max_thickness);

			if (passes_min && passes_max) {
				const Vector3 center = centers[k - 1].lerp(centers[k], t);
				const float radius = radii[k - 1] + (radii[k] - radii[k - 1]) * t;

				const Vector3 attach_dir = (out * 0.6f + right * (side_sign * 0.8f)).normalized();
				const Vector3 leaf_origin = center + attach_dir * (radius * 0.9f);

				const float yaw_jitter = (config.leaf_yaw_variation > 0.0f)
						? rng.random_float(-config.leaf_yaw_variation, config.leaf_yaw_variation)
						: 0.0f;
				const float yaw = side_sign * 0.85f + yaw_jitter;
				const Vector3 planar_fwd = (tangent * std::cos(yaw) + right * std::sin(yaw)).normalized();

				const Vector3 forward = (planar_fwd * cos_tilt + out * sin_tilt).normalized();
				const Vector3 base_up = (out * cos_tilt - planar_fwd * sin_tilt).normalized();
				const Vector3 z_axis = -forward;

				Vector3 x_axis = base_up.cross(z_axis).normalized();
				Vector3 y_axis = base_up;
				if (config.leaf_roll_variation > 0.0f) {
					const float roll = rng.random_float(
							-config.leaf_roll_variation, config.leaf_roll_variation);
					y_axis = (base_up * std::cos(roll) + x_axis * std::sin(roll)).normalized();
					x_axis = y_axis.cross(z_axis).normalized();
				}

				const float scale = (config.leaf_size_max > config.leaf_size_min)
						? rng.random_float(config.leaf_size_min, config.leaf_size_max)
						: config.leaf_size_min;

				const Basis basis(x_axis * scale, y_axis * scale, z_axis * scale);
				out_transforms.emplace_back(basis, leaf_origin);
			}

			side_sign = -side_sign;
			next_leaf_dist += spacing * rng.random_float(0.75f, 1.25f);
		}

		arc = seg_end;
	}
}

} // namespace

bool IvyMeshGenerator::build_stem_mesh(
		const std::vector<Vector3> &positions,
		const std::vector<float> &thicknesses,
		const std::vector<int32_t> &parent_indices,
		const Vector3 &wall_normal,
		const IvyMeshConfig &config,
		IvyMeshData &out_mesh) {
	out_mesh.clear();

	if (!validate_config(config)) {
		return false;
	}
	if (!wall_normal.is_finite() || wall_normal.length_squared() <= 1e-6f) {
		return false;
	}
	if (positions.empty() ||
			positions.size() != thicknesses.size() ||
			positions.size() != parent_indices.size()) {
		return false;
	}

	const int32_t count = static_cast<int32_t>(positions.size());
	float max_thickness = 0.0f;

	for (int32_t i = 0; i < count; ++i) {
		const size_t idx = static_cast<size_t>(i);
		if (!positions[idx].is_finite()) {
			return false;
		}
		if (!std::isfinite(thicknesses[idx]) || thicknesses[idx] <= 0.0f) {
			return false;
		}
		const int32_t parent = parent_indices[idx];
		if (parent < -1 || parent >= i) {
			return false;
		}
		max_thickness = std::max(max_thickness, thicknesses[idx]);
	}

	const Vector3 unit_normal = wall_normal.normalized();
	const std::vector<StemChain> chains = decompose_chains(thicknesses, parent_indices);

	for (const StemChain &chain : chains) {
		append_chain_tube(
				chain,
				positions,
				thicknesses,
				max_thickness,
				unit_normal,
				config,
				out_mesh);
	}

	return true;
}

bool IvyMeshGenerator::build_leaf_transforms(
		const std::vector<Vector3> &positions,
		const std::vector<float> &thicknesses,
		const std::vector<int32_t> &parent_indices,
		const Vector3 &wall_normal,
		const IvyMeshConfig &config,
		std::vector<Transform3D> &out_transforms) {
	out_transforms.clear();

	if (!validate_leaf_config(config)) {
		return false;
	}

	float max_thickness = 0.0f;
	if (!validate_graph_inputs(positions, thicknesses, parent_indices, wall_normal, max_thickness)) {
		return false;
	}

	if (config.leaf_density <= 0.0f) {
		return true;
	}

	const Vector3 unit_normal = wall_normal.normalized();
	const std::vector<StemChain> chains = decompose_chains(thicknesses, parent_indices);
	IvyRandom rng(static_cast<uint64_t>(config.leaf_seed));

	for (const StemChain &chain : chains) {
		append_chain_leaves(
				chain,
				positions,
				thicknesses,
				unit_normal,
				config,
				rng,
				out_transforms);
	}

	return true;
}

} // namespace ivygenerator