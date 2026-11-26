# Ivy Sim - Procedural ivy generator for Godot 4
# Copyright (C) 2026 Max Heinze
#
# SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
# Dual-licensed under GNU GPLv3 (see LICENSE) or a commercial license (see COMMERCIAL_LICENSE.md).
# Distributed WITHOUT ANY WARRANTY; see LICENSE for details.

extends RefCounted

var tests_passed := 0
var tests_failed := 0


func run_all() -> void:
	tests_passed = 0
	tests_failed = 0

	print("\n=== IvyMeshBuilder & IvyMeshSettings Tests ===")

	test_mesh_settings_properties()
	test_reject_null_settings_and_invalid_inputs()
	test_build_stem_mesh_and_leaves_from_generator()
	test_leaf_determinism_and_multimesh_upload()

	_print_test_summary()


func _run_test(test_name: String, result: bool) -> void:
	if result:
		tests_passed += 1
		print("PASS: ", test_name)
	else:
		tests_failed += 1
		print("FAIL: ", test_name)


func _print_test_summary() -> void:
	print("\n=== Test Summary ===")
	print("Passed: ", tests_passed)
	print("Failed: ", tests_failed)
	print("====================\n")


func _make_sample_generator() -> IvyGenerator:
	var image := Image.create(4, 4, false, Image.FORMAT_L8)
	image.fill(Color(1.0, 1.0, 1.0))

	var settings := IvySettings.new()
	settings.seed = 42
	settings.start_count = 2
	settings.max_steps = 15
	settings.step_length = 0.25
	settings.random_weight = 0.3
	settings.branch_probability = 0.25
	settings.thickness_growth_per_step = 0.01

	var gen := IvyGenerator.new()
	gen.settings = settings
	gen.set_wall(Vector3.ZERO, Vector3.RIGHT, Vector3.UP, 6.0, 4.0)
	gen.set_attraction_image(image)
	gen.generate()
	return gen


func test_mesh_settings_properties() -> void:
	var cfg := IvyMeshSettings.new()
	_run_test("Default radial_segments is 6", cfg.radial_segments == 6)
	_run_test("Default cap_tips is true", cfg.cap_tips == true)
	_run_test("Default leaf_density is 4.0", is_equal_approx(cfg.leaf_density, 4.0))
	_run_test("Default leaf_tilt_degrees is 30.0", is_equal_approx(cfg.leaf_tilt_degrees, 30.0))

	cfg.radial_segments = 8
	cfg.radius_scale = 0.6
	cfg.wall_offset_ratio = 0.75
	cfg.uv_v_scale = 2.0
	cfg.cap_tips = false
	cfg.leaf_seed = 99
	cfg.leaf_density = 6.5
	cfg.leaf_min_thickness = 0.05
	cfg.leaf_max_thickness = 0.3
	cfg.leaf_size_min = 0.1
	cfg.leaf_size_max = 0.2
	cfg.leaf_tilt_degrees = 45.0
	cfg.leaf_yaw_variation_degrees = 35.0
	cfg.leaf_roll_variation_degrees = 15.0

	_run_test("MeshSettings radial_segments round-trip", cfg.radial_segments == 8)
	_run_test("MeshSettings cap_tips round-trip", cfg.cap_tips == false)
	_run_test("MeshSettings leaf_density round-trip", is_equal_approx(cfg.leaf_density, 6.5))
	_run_test("MeshSettings leaf_tilt_degrees round-trip", is_equal_approx(cfg.leaf_tilt_degrees, 45.0))


func test_reject_null_settings_and_invalid_inputs() -> void:
	var builder := IvyMeshBuilder.new()
	var pos := PackedVector3Array([Vector3.ZERO, Vector3.UP])
	var thick := PackedFloat32Array([0.1, 0.1])
	var parents := PackedInt32Array([-1, 0])

	_run_test("Null settings returns null stem mesh",
		builder.build_stem_mesh(pos, thick, parents, Vector3.BACK) == null)
	_run_test("Null settings returns empty leaf transforms",
		builder.build_leaf_transforms(pos, thick, parents, Vector3.BACK).is_empty())

	builder.settings = IvyMeshSettings.new()
	_run_test("Zero wall normal returns null stem mesh",
		builder.build_stem_mesh(pos, thick, parents, Vector3.ZERO) == null)
	_run_test("Empty arrays return null stem mesh",
		builder.build_stem_mesh(PackedVector3Array(), PackedFloat32Array(), PackedInt32Array(), Vector3.BACK) == null)


func test_build_stem_mesh_and_leaves_from_generator() -> void:
	var gen := _make_sample_generator()
	var builder := IvyMeshBuilder.new()
	builder.settings = IvyMeshSettings.new()

	var mesh := builder.build_stem_mesh(
		gen.get_point_positions(),
		gen.get_point_thicknesses(),
		gen.get_point_parent_indices(),
		Vector3.BACK)

	_run_test("Stem ArrayMesh created", mesh != null)
	if mesh == null:
		return

	_run_test("Stem ArrayMesh has 1 surface", mesh.get_surface_count() == 1)
	_run_test("Stem surface uses PRIMITIVE_TRIANGLES",
		mesh.surface_get_primitive_type(0) == Mesh.PRIMITIVE_TRIANGLES)

	var arrays := mesh.surface_get_arrays(0)
	var verts: PackedVector3Array = arrays[Mesh.ARRAY_VERTEX]
	var normals: PackedVector3Array = arrays[Mesh.ARRAY_NORMAL]
	var uvs: PackedVector2Array = arrays[Mesh.ARRAY_TEX_UV]
	var colors: PackedColorArray = arrays[Mesh.ARRAY_COLOR]
	var indices: PackedInt32Array = arrays[Mesh.ARRAY_INDEX]

	_run_test("Stem mesh has non-empty vertices and indices",
		verts.size() > 0 and indices.size() > 0 and indices.size() % 3 == 0)
	_run_test("Stem vertex attribute buffers match vertex count",
		normals.size() == verts.size() and uvs.size() == verts.size() and colors.size() == verts.size())

	var leaves := builder.build_leaf_transforms(
		gen.get_point_positions(),
		gen.get_point_thicknesses(),
		gen.get_point_parent_indices(),
		Vector3.BACK)
	_run_test("Leaf transforms generated from solver graph", leaves.size() > 0)
	if leaves.size() > 0:
		var first: Transform3D = leaves[0]
		_run_test("Leaf transform is right-handed and in front of wall",
			first.basis.determinant() > 0.0 and first.origin.z > 0.0)


func test_leaf_determinism_and_multimesh_upload() -> void:
	var gen := _make_sample_generator()
	var pos := gen.get_point_positions()
	var thick := gen.get_point_thicknesses()
	var parents := gen.get_point_parent_indices()

	var builder := IvyMeshBuilder.new()
	var cfg := IvyMeshSettings.new()
	cfg.leaf_seed = 77
	builder.settings = cfg

	var leaves_a := builder.build_leaf_transforms(pos, thick, parents, Vector3.BACK)
	var leaves_b := builder.build_leaf_transforms(pos, thick, parents, Vector3.BACK)

	cfg.leaf_seed = 88
	var leaves_c := builder.build_leaf_transforms(pos, thick, parents, Vector3.BACK)

	_run_test("Identical leaf_seed produces identical transforms", leaves_a == leaves_b and leaves_a.size() > 0)
	_run_test("Different leaf_seed produces different transforms", leaves_a != leaves_c)

	# Verify transforms upload cleanly to a MultiMesh.
	var mm := MultiMesh.new()
	mm.transform_format = MultiMesh.TRANSFORM_3D
	mm.mesh = QuadMesh.new()
	mm.instance_count = leaves_a.size()
	for i in range(leaves_a.size()):
		mm.set_instance_transform(i, leaves_a[i])

	var all_valid := true
	for xf: Transform3D in leaves_a:
		if not xf.is_finite() or xf.basis.determinant() <= 0.0:
			all_valid = false
			break
	
	var readback_ok := (DisplayServer.get_name() == "headless") or mm.get_instance_transform(0).is_equal_approx(leaves_a[0])
	
	_run_test("MultiMesh accepts all valid leaf transforms",
	all_valid and mm.instance_count == leaves_a.size() and readback_ok)
	cfg.leaf_density = 0.0
	_run_test("Zero leaf_density yields empty transforms",
		builder.build_leaf_transforms(pos, thick, parents, Vector3.BACK).is_empty())
