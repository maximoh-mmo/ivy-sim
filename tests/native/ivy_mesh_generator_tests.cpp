/*
 * Copyright (c) 2026 Max Heinze. All rights reserved.
 *
 * Licensed for non-commercial use with attribution only.
 * Commercial use is strictly prohibited.
 */

/**
 * @file ivy_mesh_generator_tests.cpp
 * @brief Implementation of IvyMeshGenerator unit tests.
 */

#include "ivy_mesh_generator_tests.h"

#include "environment/ivy_surface_sampler.h"
#include "environment/ivy_wall.h"
#include "growth/ivy_growth_solver.h"
#include "mesh/ivy_mesh_generator.h"
#include "random/ivy_random.h"
#include "settings/ivy_growth_config.h"

#include <cmath>
#include <iostream>
#include <limits>
#include <vector>

namespace ivygenerator {
using namespace godot;

namespace {

bool verify_mesh_integrity_and_cw_winding(const IvyMeshData &mesh) {
	const size_t vcount = mesh.vertices.size();
	if (mesh.normals.size() != vcount ||
			mesh.uvs.size() != vcount ||
			mesh.colors.size() != vcount) {
		return false;
	}
	if (mesh.indices.size() % 3 != 0) {
		return false;
	}

	for (size_t i = 0; i < vcount; ++i) {
		if (!mesh.vertices[i].is_finite() ||
				!mesh.normals[i].is_finite() ||
				!mesh.uvs[i].is_finite()) {
			return false;
		}
		if (std::abs(mesh.normals[i].length() - 1.0f) > 0.001f) {
			return false;
		}
	}

	for (size_t t = 0; t < mesh.indices.size(); t += 3) {
		const int32_t i0 = mesh.indices[t];
		const int32_t i1 = mesh.indices[t + 1];
		const int32_t i2 = mesh.indices[t + 2];
		if (i0 < 0 || i1 < 0 || i2 < 0 ||
				static_cast<size_t>(i0) >= vcount ||
				static_cast<size_t>(i1) >= vcount ||
				static_cast<size_t>(i2) >= vcount) {
			return false;
		}

		const Vector3 &v0 = mesh.vertices[static_cast<size_t>(i0)];
		const Vector3 &v1 = mesh.vertices[static_cast<size_t>(i1)];
		const Vector3 &v2 = mesh.vertices[static_cast<size_t>(i2)];
		const Vector3 cross = (v1 - v0).cross(v2 - v0);
		if (cross.length_squared() <= 1e-12f) {
			continue;
		}

		// Godot CW front-face convention: (v1 - v0) x (v2 - v0) points inward (opposite vertex normals).
		const Vector3 avg_normal =
				mesh.normals[static_cast<size_t>(i0)] +
				mesh.normals[static_cast<size_t>(i1)] +
				mesh.normals[static_cast<size_t>(i2)];
		if (cross.dot(avg_normal) >= 0.0f) {
			return false;
		}
	}

	return true;
}

} // namespace

bool IvyMeshGeneratorTests::run_test(
		const char *name,
		bool (IvyMeshGeneratorTests::*test)()) {
	std::cout << "Running: " << name << '\n';

	const bool result = (this->*test)();

	if (result) {
		++m_passed;
		std::cout << "    PASS\n";
	} else {
		++m_failed;
		std::cout << "    FAIL\n";
	}

	return result;
}

bool IvyMeshGeneratorTests::expect(bool condition, const char *message) {
	if (!condition) {
		std::cout << "    FAILED: " << message << '\n';
	}

	return condition;
}

bool IvyMeshGeneratorTests::test_reject_invalid_inputs_and_clears_output() {
	const std::vector<Vector3> valid_pos = { Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, 1.0f, 0.0f) };
	const std::vector<float> valid_thick = { 0.2f, 0.1f };
	const std::vector<int32_t> valid_parents = { -1, 0 };
	const Vector3 normal(0.0f, 0.0f, 1.0f);
	const IvyMeshConfig valid_cfg;

	IvyMeshData mesh;
	if (!expect(IvyMeshGenerator::build_stem_mesh(valid_pos, valid_thick, valid_parents, normal, valid_cfg, mesh),
				"Valid baseline input should succeed") ||
			!expect(!mesh.is_empty(), "Baseline mesh should be populated")) {
		return false;
	}

	// Empty input should fail and clear previously populated output.
	if (!expect(!IvyMeshGenerator::build_stem_mesh({}, {}, {}, normal, valid_cfg, mesh),
				"Empty input should be rejected") ||
			!expect(mesh.is_empty(), "Failed call should clear stale mesh buffers")) {
		return false;
	}

	// Mismatched sizes.
	if (!expect(!IvyMeshGenerator::build_stem_mesh(valid_pos, { 0.2f }, valid_parents, normal, valid_cfg, mesh),
				"Mismatched thicknesses size should be rejected")) {
		return false;
	}

	// Invalid parent index (parent >= self or < -1).
	if (!expect(!IvyMeshGenerator::build_stem_mesh(valid_pos, valid_thick, { -1, 1 }, normal, valid_cfg, mesh),
				"Self-parent index should be rejected") ||
			!expect(!IvyMeshGenerator::build_stem_mesh(valid_pos, valid_thick, { -2, 0 }, normal, valid_cfg, mesh),
					"Parent index < -1 should be rejected")) {
		return false;
	}

	// Non-positive or NaN thickness.
	if (!expect(!IvyMeshGenerator::build_stem_mesh(valid_pos, { 0.2f, 0.0f }, valid_parents, normal, valid_cfg, mesh),
				"Zero thickness should be rejected") ||
			!expect(!IvyMeshGenerator::build_stem_mesh(
							valid_pos, { 0.2f, std::numeric_limits<float>::quiet_NaN() }, valid_parents, normal, valid_cfg, mesh),
					"NaN thickness should be rejected")) {
		return false;
	}

	// Zero wall normal.
	if (!expect(!IvyMeshGenerator::build_stem_mesh(valid_pos, valid_thick, valid_parents, Vector3(), valid_cfg, mesh),
				"Zero wall normal should be rejected")) {
		return false;
	}

	// Invalid config fields.
	IvyMeshConfig bad_segs = valid_cfg;
	bad_segs.radial_segments = 2;
	IvyMeshConfig bad_scale = valid_cfg;
	bad_scale.radius_scale = 0.0f;
	IvyMeshConfig bad_offset = valid_cfg;
	bad_offset.wall_offset_ratio = -0.1f;
	IvyMeshConfig bad_uv = valid_cfg;
	bad_uv.uv_v_scale = 0.0f;

	return expect(!IvyMeshGenerator::build_stem_mesh(valid_pos, valid_thick, valid_parents, normal, bad_segs, mesh),
				   "radial_segments < 3 should be rejected") &&
			expect(!IvyMeshGenerator::build_stem_mesh(valid_pos, valid_thick, valid_parents, normal, bad_scale, mesh),
					"Non-positive radius_scale should be rejected") &&
			expect(!IvyMeshGenerator::build_stem_mesh(valid_pos, valid_thick, valid_parents, normal, bad_offset, mesh),
					"Negative wall_offset_ratio should be rejected") &&
			expect(!IvyMeshGenerator::build_stem_mesh(valid_pos, valid_thick, valid_parents, normal, bad_uv, mesh),
					"Non-positive uv_v_scale should be rejected");
}

bool IvyMeshGeneratorTests::test_isolated_root_produces_empty_mesh() {
	IvyMeshData mesh;
	const IvyMeshConfig cfg;
	const bool ok = IvyMeshGenerator::build_stem_mesh(
			{ Vector3(1.0f, 0.0f, 0.0f) },
			{ 0.1f },
			{ -1 },
			Vector3(0.0f, 0.0f, 1.0f),
			cfg,
			mesh);

	return expect(ok, "Single isolated root should succeed gracefully") &&
			expect(mesh.is_empty(), "Single isolated root with no segments should produce empty mesh");
}

bool IvyMeshGeneratorTests::test_single_segment_geometry_uvs_and_winding() {
	const std::vector<Vector3> pos = { Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, 2.0f, 0.0f) };
	const std::vector<float> thick = { 0.2f, 0.2f };
	const std::vector<int32_t> parents = { -1, 0 };
	const Vector3 normal(0.0f, 0.0f, 1.0f);

	IvyMeshConfig cfg;
	cfg.radial_segments = 6;
	cfg.radius_scale = 0.5f;
	cfg.wall_offset_ratio = 0.0f;
	cfg.uv_v_scale = 1.5f;
	cfg.cap_tips = false;

	IvyMeshData uncapped;
	if (!expect(IvyMeshGenerator::build_stem_mesh(pos, thick, parents, normal, cfg, uncapped),
				"Uncapped single segment should succeed")) {
		return false;
	}

	const size_t stride = static_cast<size_t>(cfg.radial_segments + 1);
	if (!expect(uncapped.vertices.size() == 2 * stride, "Uncapped segment should have 2 * (radial_segments + 1) vertices") ||
			!expect(uncapped.indices.size() == static_cast<size_t>(cfg.radial_segments * 6),
					"Uncapped segment should have radial_segments * 6 indices") ||
			!expect(verify_mesh_integrity_and_cw_winding(uncapped),
					"Uncapped mesh should pass buffer, unit-normal, and CW winding checks")) {
		return false;
	}

	// Check seam UV wrapping [0, 1] and arc-length V scaling (length 2.0 * uv_v_scale 1.5 = 3.0).
	if (!expect(std::abs(uncapped.uvs[0].x - 0.0f) < 0.0001f &&
						std::abs(uncapped.uvs[stride - 1].x - 1.0f) < 0.0001f &&
						std::abs(uncapped.uvs[0].y - 0.0f) < 0.0001f &&
						std::abs(uncapped.uvs[stride].y - 3.0f) < 0.001f,
				"UVs should wrap u in [0, 1] and scale v by arc_length * uv_v_scale")) {
		return false;
	}

	// With cap_tips enabled: +1 apex vertex, +radial_segments * 3 indices.
	cfg.cap_tips = true;
	IvyMeshData capped;
	if (!expect(IvyMeshGenerator::build_stem_mesh(pos, thick, parents, normal, cfg, capped),
				"Capped single segment should succeed")) {
		return false;
	}

	return expect(capped.vertices.size() == 2 * stride + 1, "Capped segment should add 1 apex vertex") &&
			expect(capped.indices.size() == static_cast<size_t>(cfg.radial_segments * 9),
					"Capped segment should have radial_segments * 9 indices") &&
			expect(verify_mesh_integrity_and_cw_winding(capped),
					"Capped mesh should pass buffer, unit-normal, and CW winding checks");
}

bool IvyMeshGeneratorTests::test_wall_adhesion_offset() {
	const std::vector<Vector3> pos = { Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, 2.0f, 0.0f) };
	const std::vector<float> thick = { 0.4f, 0.2f };
	const std::vector<int32_t> parents = { -1, 0 };

	IvyMeshConfig cfg;
	cfg.radial_segments = 6;
	cfg.radius_scale = 0.5f;
	cfg.wall_offset_ratio = 0.8f;
	cfg.cap_tips = false;

	IvyMeshData mesh;
	if (!expect(IvyMeshGenerator::build_stem_mesh(pos, thick, parents, Vector3(0.0f, 0.0f, 1.0f), cfg, mesh),
				"Offset stem mesh should succeed")) {
		return false;
	}

	const int32_t segs = cfg.radial_segments;
	const int32_t stride = segs + 1;
	Vector3 ring0_center;
	Vector3 ring1_center;
	for (int32_t s = 0; s < segs; ++s) {
		ring0_center += mesh.vertices[static_cast<size_t>(s)];
		ring1_center += mesh.vertices[static_cast<size_t>(stride + s)];
	}
	ring0_center /= static_cast<float>(segs);
	ring1_center /= static_cast<float>(segs);

	// r0 = 0.2 -> offset = 0.16; r1 = 0.1 -> offset = 0.08.
	return expect(ring0_center.is_equal_approx(Vector3(0.0f, 0.0f, 0.16f)),
				   "Ring 0 center should shift along wall_normal by r0 * wall_offset_ratio") &&
			expect(ring1_center.is_equal_approx(Vector3(0.0f, 2.0f, 0.08f)),
					"Ring 1 center should shift along wall_normal by r1 * wall_offset_ratio");
}

bool IvyMeshGeneratorTests::test_vertex_color_channels() {
	const std::vector<Vector3> pos = {
		Vector3(0.0f, 0.0f, 0.0f),
		Vector3(0.0f, 1.0f, 0.0f),
		Vector3(0.0f, 2.0f, 0.0f)
	};
	const std::vector<float> thick = { 0.4f, 0.2f, 0.1f };
	const std::vector<int32_t> parents = { -1, 0, 1 };

	IvyMeshConfig cfg;
	cfg.radial_segments = 4;
	cfg.wall_offset_ratio = 0.0f;
	cfg.cap_tips = false;

	IvyMeshData mesh;
	if (!expect(IvyMeshGenerator::build_stem_mesh(pos, thick, parents, Vector3(0.0f, 0.0f, 1.0f), cfg, mesh),
				"Color channel test mesh should succeed")) {
		return false;
	}

	const size_t stride = static_cast<size_t>(cfg.radial_segments + 1);
	const Color &c_root = mesh.colors[0];
	const Color &c_mid = mesh.colors[stride];
	const Color &c_tip = mesh.colors[2 * stride];

	return expect(std::abs(c_root.r - 1.0f) < 0.001f &&
						   std::abs(c_mid.r - 0.5f) < 0.001f &&
						   std::abs(c_tip.r - 0.25f) < 0.001f,
				   "R channel should encode thickness / max_thickness") &&
			expect(std::abs(c_root.g - 0.0f) < 0.001f &&
							std::abs(c_mid.g - 0.5f) < 0.001f &&
							std::abs(c_tip.g - 1.0f) < 0.001f,
					"G channel should encode normalized chain progress [0, 1]") &&
			expect(std::abs(c_root.b - 0.0f) < 0.001f &&
							std::abs(c_tip.b - 0.0f) < 0.001f,
					"B channel should be 0 on primary root chain") &&
			expect(std::abs(c_root.a - 1.0f) < 0.001f &&
							std::abs(c_tip.a - 1.0f) < 0.001f,
					"A channel should be 1.0");
}

bool IvyMeshGeneratorTests::test_branching_y_junction_and_branch_order() {
	// Root (0) -> Fork (1) -> Thick primary child (2) and thin side branch (3).
	const std::vector<Vector3> pos = {
		Vector3(0.0f, 0.0f, 0.0f),
		Vector3(0.0f, 1.0f, 0.0f),
		Vector3(0.0f, 2.0f, 0.0f),
		Vector3(1.0f, 2.0f, 0.0f)
	};
	const std::vector<float> thick = { 0.4f, 0.3f, 0.25f, 0.1f };
	const std::vector<int32_t> parents = { -1, 0, 1, 1 };

	IvyMeshConfig cfg;
	cfg.radial_segments = 6;
	cfg.radius_scale = 0.5f;
	cfg.wall_offset_ratio = 0.85f;
	cfg.cap_tips = true;

	IvyMeshData mesh;
	if (!expect(IvyMeshGenerator::build_stem_mesh(pos, thick, parents, Vector3(0.0f, 0.0f, 1.0f), cfg, mesh),
				"Branching Y-junction mesh should succeed")) {
		return false;
	}

	const size_t stride = static_cast<size_t>(cfg.radial_segments + 1);
	const size_t main_verts = 3 * stride + 1;
	const size_t branch_verts = 2 * stride + 1;
	const size_t expected_indices = static_cast<size_t>((2 * 6 + 3 + 1 * 6 + 3) * cfg.radial_segments);

	if (!expect(mesh.vertices.size() == main_verts + branch_verts,
				"Y-junction should emit 3-ring primary chain + 2-ring side branch") ||
			!expect(mesh.indices.size() == expected_indices,
					"Y-junction index count should match both capped chains") ||
			!expect(verify_mesh_integrity_and_cw_winding(mesh),
					"Y-junction mesh should pass integrity and CW winding checks")) {
		return false;
	}

	// Side branch ring 0 should seat at parent node 1's offset center (z = 0.3 * 0.5 * 0.85 = 0.1275).
	Vector3 branch_base_center;
	for (int32_t s = 0; s < cfg.radial_segments; ++s) {
		branch_base_center += mesh.vertices[main_verts + static_cast<size_t>(s)];
	}
	branch_base_center /= static_cast<float>(cfg.radial_segments);

	return expect(std::abs(mesh.colors[0].b - 0.0f) < 0.001f,
				   "Primary stem should have branch_order color B == 0.0") &&
			expect(std::abs(mesh.colors[main_verts].b - 0.25f) < 0.001f,
					"First-order side branch should have branch_order color B == 0.25") &&
			expect(branch_base_center.is_equal_approx(Vector3(0.0f, 1.0f, 0.1275f)),
					"Side branch base ring should seat at fork parent's offset centerline");
}

bool IvyMeshGeneratorTests::test_end_to_end_solver_graph_to_mesh() {
	IvyWall wall;
	wall.origin = Vector3(0.0f, 0.0f, 0.0f);
	wall.axis_u = Vector3(1.0f, 0.0f, 0.0f);
	wall.axis_v = Vector3(0.0f, 1.0f, 0.0f);
	wall.width = 6.0f;
	wall.height = 4.0f;

	IvySurfaceSampler sampler;
	sampler.set_pixel_data({ 1.0f }, 1, 1);

	IvyGrowthConfig growth_cfg;
	growth_cfg.step_length = 0.2f;
	growth_cfg.random_weight = 0.3f;
	growth_cfg.branch_probability = 0.35f;
	growth_cfg.min_branch_interval = 1;
	growth_cfg.thickness_growth_per_step = 0.01f;

	IvyRandom rng(42);
	IvyGrowthSolver solver;
	const std::vector<Vector3> starts = {
		Vector3(1.5f, 0.0f, 0.0f),
		Vector3(4.5f, 0.0f, 0.0f)
	};

	if (!expect(solver.initialize(wall, sampler, growth_cfg, rng, starts),
				"Solver should initialize for end-to-end mesh test")) {
		return false;
	}

	for (int i = 0; i < 10; ++i) {
		if (solver.step() == GrowthStepResult::ERROR) {
			return expect(false, "Solver step should not error");
		}
	}

	const IvyGrowthGraph &graph = solver.get_graph();
	const int32_t n = graph.get_point_count();
	std::vector<Vector3> positions;
	std::vector<float> thicknesses;
	std::vector<int32_t> parents;
	positions.reserve(static_cast<size_t>(n));
	thicknesses.reserve(static_cast<size_t>(n));
	parents.reserve(static_cast<size_t>(n));

	for (int32_t i = 0; i < n; ++i) {
		const IvyPoint *pt = graph.get_point(i);
		positions.push_back(pt->position);
		thicknesses.push_back(pt->thickness);
		parents.push_back(pt->parent_index);
	}

	IvyMeshData mesh;
	const IvyMeshConfig mesh_cfg;
	const bool ok = IvyMeshGenerator::build_stem_mesh(
			positions, thicknesses, parents, wall.get_normal(), mesh_cfg, mesh);

	return expect(ok, "End-to-end stem mesh generation should succeed") &&
			expect(!mesh.is_empty(), "End-to-end stem mesh should contain geometry") &&
			expect(verify_mesh_integrity_and_cw_winding(mesh),
					"End-to-end stem mesh should pass all integrity and CW winding checks");
}

bool IvyMeshGeneratorTests::run_all() {
	m_passed = 0;
	m_failed = 0;

	std::cout << "\n=== IvyMeshGenerator Unit Tests ===\n\n";

	run_test("Reject invalid inputs and clears output",
			&IvyMeshGeneratorTests::test_reject_invalid_inputs_and_clears_output);

	run_test("Isolated root produces empty mesh",
			&IvyMeshGeneratorTests::test_isolated_root_produces_empty_mesh);

	run_test("Single segment geometry, UVs, and CW winding",
			&IvyMeshGeneratorTests::test_single_segment_geometry_uvs_and_winding);

	run_test("Wall adhesion offset",
			&IvyMeshGeneratorTests::test_wall_adhesion_offset);

	run_test("Vertex color channels",
			&IvyMeshGeneratorTests::test_vertex_color_channels);

	run_test("Branching Y-junction and branch order",
			&IvyMeshGeneratorTests::test_branching_y_junction_and_branch_order);

	run_test("End-to-end solver graph to mesh",
			&IvyMeshGeneratorTests::test_end_to_end_solver_graph_to_mesh);

	run_test("Reject invalid leaf inputs and clears output",
			&IvyMeshGeneratorTests::test_leaf_reject_invalid_config_and_zero_density);

	run_test("Leaf orientation handedness and scale",
			&IvyMeshGeneratorTests::test_leaf_orientation_handedness_and_scale);

	run_test("Leaf thickness filtering",
			&IvyMeshGeneratorTests::test_leaf_thickness_filtering);

	run_test("Leaf determinism across seeds",
			&IvyMeshGeneratorTests::test_leaf_determinism_across_seeds);

	run_test("End-to-end leaf solver",
			&IvyMeshGeneratorTests::test_leaf_end_to_end_with_solver);

	std::cout << "\n=== Test Summary ===\n";
	std::cout << "Passed: " << m_passed << '\n';
	std::cout << "Failed: " << m_failed << '\n';

	return m_failed == 0;
}

bool IvyMeshGeneratorTests::test_leaf_reject_invalid_config_and_zero_density() {
	const std::vector<Vector3> pos = { Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, 2.0f, 0.0f) };
	const std::vector<float> thick = { 0.1f, 0.1f };
	const std::vector<int32_t> parents = { -1, 0 };
	const Vector3 normal(0.0f, 0.0f, 1.0f);

	IvyMeshConfig cfg;
	std::vector<Transform3D> leaves;
	if (!expect(IvyMeshGenerator::build_leaf_transforms(pos, thick, parents, normal, cfg, leaves) && !leaves.empty(),
				"Valid leaf generation should produce transforms")) {
		return false;
	}

	IvyMeshConfig neg_density = cfg;
	neg_density.leaf_density = -1.0f;
	if (!expect(!IvyMeshGenerator::build_leaf_transforms(pos, thick, parents, normal, neg_density, leaves) && leaves.empty(),
				"Negative leaf_density should fail and clear stale output")) {
		return false;
	}

	IvyMeshConfig bad_range = cfg;
	bad_range.leaf_size_min = 0.2f;
	bad_range.leaf_size_max = 0.1f;
	IvyMeshConfig bad_thick = cfg;
	bad_thick.leaf_min_thickness = 0.3f;
	bad_thick.leaf_max_thickness = 0.2f;
	IvyMeshConfig bad_tilt = cfg;
	bad_tilt.leaf_tilt_radians = 2.0f;

	if (!expect(!IvyMeshGenerator::build_leaf_transforms(pos, thick, parents, normal, bad_range, leaves),
				"leaf_size_max < leaf_size_min should be rejected") ||
			!expect(!IvyMeshGenerator::build_leaf_transforms(pos, thick, parents, normal, bad_thick, leaves),
					"leaf_max_thickness < leaf_min_thickness should be rejected") ||
			!expect(!IvyMeshGenerator::build_leaf_transforms(pos, thick, parents, normal, bad_tilt, leaves),
					"leaf_tilt_radians > pi/2 should be rejected")) {
		return false;
	}

	IvyMeshConfig zero_density = cfg;
	zero_density.leaf_density = 0.0f;
	return expect(IvyMeshGenerator::build_leaf_transforms(pos, thick, parents, normal, zero_density, leaves),
				   "Zero leaf_density should succeed") &&
			expect(leaves.empty(), "Zero leaf_density should produce 0 transforms");
}

bool IvyMeshGeneratorTests::test_leaf_orientation_handedness_and_scale() {
	const std::vector<Vector3> pos = { Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, 3.0f, 0.0f) };
	const std::vector<float> thick = { 0.1f, 0.1f };
	const std::vector<int32_t> parents = { -1, 0 };
	const Vector3 normal(0.0f, 0.0f, 1.0f);

	IvyMeshConfig cfg;
	cfg.leaf_density = 5.0f;
	cfg.leaf_size_min = 0.1f;
	cfg.leaf_size_max = 0.2f;
	cfg.leaf_tilt_radians = 0.5f;
	cfg.leaf_roll_variation = 0.0f;

	std::vector<Transform3D> leaves;
	if (!expect(IvyMeshGenerator::build_leaf_transforms(pos, thick, parents, normal, cfg, leaves),
				"Leaf transform generation should succeed") ||
			!expect(leaves.size() >= 8, "Should generate multiple leaves along 3-unit stem")) {
		return false;
	}

	for (const Transform3D &xf : leaves) {
		const Vector3 x_col = xf.basis.get_column(0);
		const Vector3 y_col = xf.basis.get_column(1);
		const Vector3 z_col = xf.basis.get_column(2);
		const float sx = static_cast<float>(x_col.length());
		const float sy = static_cast<float>(y_col.length());
		const float sz = static_cast<float>(z_col.length());

		if (!expect(xf.origin.is_finite() && xf.origin.z > 0.0f,
					"Leaf origin should be finite and on the front side of the wall")) {
			return false;
		}
		if (!expect(sx >= cfg.leaf_size_min - 0.001f && sx <= cfg.leaf_size_max + 0.001f &&
							std::abs(sx - sy) < 0.001f && std::abs(sx - sz) < 0.001f,
					"Leaf scale should be uniform within [leaf_size_min, leaf_size_max]")) {
			return false;
		}
		if (!expect(xf.basis.determinant() > 0.0f,
					"Leaf basis should be strictly right-handed (positive determinant)")) {
			return false;
		}

		const Vector3 blade_fwd = (-z_col).normalized();
		const Vector3 leaf_up = y_col.normalized();
		if (!expect(blade_fwd.dot(normal) > 0.0f,
					"Positive leaf_tilt_radians should lift blade (-Z) away from wall") ||
				!expect(leaf_up.dot(normal) > 0.0f,
						"Leaf face normal (+Y) should point outward toward wall_normal")) {
			return false;
		}
	}

	return true;
}

bool IvyMeshGeneratorTests::test_leaf_thickness_filtering() {
	// Thick trunk (0 -> 1: 0.4) -> living shoot (1 -> 2: 0.1) -> withered twig (2 -> 3: 0.02).
	const std::vector<Vector3> pos = {
		Vector3(0.0f, 0.0f, 0.0f),
		Vector3(0.0f, 2.0f, 0.0f),
		Vector3(0.0f, 4.0f, 0.0f),
		Vector3(0.0f, 6.0f, 0.0f)
	};
	const std::vector<float> thick = { 0.4f, 0.4f, 0.1f, 0.02f };
	const std::vector<int32_t> parents = { -1, 0, 1, 2 };
	const Vector3 normal(0.0f, 0.0f, 1.0f);

	IvyMeshConfig cfg;
	cfg.leaf_density = 6.0f;
	cfg.leaf_min_thickness = 0.08f;
	cfg.leaf_max_thickness = 0.25f;

	std::vector<Transform3D> leaves;
	if (!expect(IvyMeshGenerator::build_leaf_transforms(pos, thick, parents, normal, cfg, leaves),
				"Filtered leaf generation should succeed") ||
			!expect(!leaves.empty(), "Living middle segment should still receive leaves")) {
		return false;
	}

	for (const Transform3D &xf : leaves) {
		if (!expect(xf.origin.y > 2.0f && xf.origin.y < 5.5f,
					"Leaves should only spawn where interpolated thickness is within [min, max]")) {
			return false;
		}
	}

	return true;
}

bool IvyMeshGeneratorTests::test_leaf_determinism_across_seeds() {
	const std::vector<Vector3> pos = { Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, 3.0f, 0.0f) };
	const std::vector<float> thick = { 0.1f, 0.1f };
	const std::vector<int32_t> parents = { -1, 0 };
	const Vector3 normal(0.0f, 0.0f, 1.0f);

	IvyMeshConfig cfg_a;
	cfg_a.leaf_seed = 123;
	IvyMeshConfig cfg_b = cfg_a;
	IvyMeshConfig cfg_c = cfg_a;
	cfg_c.leaf_seed = 456;

	std::vector<Transform3D> a, b, c;
	IvyMeshGenerator::build_leaf_transforms(pos, thick, parents, normal, cfg_a, a);
	IvyMeshGenerator::build_leaf_transforms(pos, thick, parents, normal, cfg_b, b);
	IvyMeshGenerator::build_leaf_transforms(pos, thick, parents, normal, cfg_c, c);

	if (!expect(a.size() == b.size() && !a.empty(), "Identical seeds should produce equal leaf counts")) {
		return false;
	}
	for (size_t i = 0; i < a.size(); ++i) {
		if (!expect(a[i].is_equal_approx(b[i]), "Identical seeds should produce identical leaf transforms")) {
			return false;
		}
	}

	bool any_diff = (a.size() != c.size());
	for (size_t i = 0; !any_diff && i < a.size(); ++i) {
		if (!a[i].is_equal_approx(c[i])) {
			any_diff = true;
		}
	}
	return expect(any_diff, "Different leaf seeds should produce different placements");
}

bool IvyMeshGeneratorTests::test_leaf_end_to_end_with_solver() {
	IvyWall wall;
	wall.width = 6.0f;
	wall.height = 4.0f;

	IvySurfaceSampler sampler;
	sampler.set_pixel_data({ 1.0f }, 1, 1);

	IvyGrowthConfig growth_cfg;
	growth_cfg.step_length = 0.2f;
	growth_cfg.random_weight = 0.3f;
	growth_cfg.branch_probability = 0.35f;

	IvyRandom rng(42);
	IvyGrowthSolver solver;
	solver.initialize(wall, sampler, growth_cfg, rng, { Vector3(3.0f, 0.0f, 0.0f) });
	for (int i = 0; i < 10; ++i) {
		solver.step();
	}

	const IvyGrowthGraph &graph = solver.get_graph();
	const int32_t n = graph.get_point_count();
	std::vector<Vector3> positions;
	std::vector<float> thicknesses;
	std::vector<int32_t> parents;
	positions.reserve(static_cast<size_t>(n));
	thicknesses.reserve(static_cast<size_t>(n));
	parents.reserve(static_cast<size_t>(n));
	for (int32_t i = 0; i < n; ++i) {
		const IvyPoint *pt = graph.get_point(i);
		positions.push_back(pt->position);
		thicknesses.push_back(pt->thickness);
		parents.push_back(pt->parent_index);
	}

	std::vector<Transform3D> leaves;
	const IvyMeshConfig mesh_cfg;
	const bool ok = IvyMeshGenerator::build_leaf_transforms(
			positions, thicknesses, parents, wall.get_normal(), mesh_cfg, leaves);

	return expect(ok && !leaves.empty(), "End-to-end solver graph should generate leaf transforms");
}

} // namespace ivygenerator