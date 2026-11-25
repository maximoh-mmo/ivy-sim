# Copyright (c) 2026 Max Heinze. All rights reserved.
#
# Licensed for non-commercial use with attribution only.
# Commercial use is strictly prohibited.

class_name IvyDebugRenderer
extends RefCounted


static func build_ribbon_mesh(
		positions: PackedVector3Array,
		thicknesses: PackedFloat32Array,
		parent_indices: PackedInt32Array,
		wall_normal: Vector3,
		initial_thickness: float,
		surface_offset: float = 0.015) -> ArrayMesh:
	var st := SurfaceTool.new()
	st.begin(Mesh.PRIMITIVE_TRIANGLES)

	var normal := wall_normal.normalized()
	var count := positions.size()

	for i in range(count):
		var parent_idx := parent_indices[i]
		if parent_idx < 0 or parent_idx >= count:
			continue

		var p0 := positions[parent_idx] + normal * surface_offset
		var p1 := positions[i] + normal * surface_offset
		var seg := p1 - p0
		if seg.length_squared() <= 0.000001:
			continue

		var side := normal.cross(seg.normalized()).normalized()
		var r0 := thicknesses[parent_idx] * 0.5
		var r1 := thicknesses[i] * 0.5

		var c0 := _thickness_color(thicknesses[parent_idx], initial_thickness)
		var c1 := _thickness_color(thicknesses[i], initial_thickness)

		var v00 := p0 - side * r0
		var v01 := p0 + side * r0
		var v10 := p1 - side * r1
		var v11 := p1 + side * r1

		st.set_normal(normal)
		st.set_color(c0); st.add_vertex(v00)
		st.set_color(c0); st.add_vertex(v01)
		st.set_color(c1); st.add_vertex(v11)

		st.set_normal(normal)
		st.set_color(c0); st.add_vertex(v00)
		st.set_color(c1); st.add_vertex(v11)
		st.set_color(c1); st.add_vertex(v10)

	return st.commit()


static func _thickness_color(thickness: float, initial_thickness: float) -> Color:
	if thickness < initial_thickness - 0.0005:
		return Color(0.55, 0.42, 0.25) # Withered dried twig (tan/brown)
	var maturity := clampf((thickness - initial_thickness) / maxf(initial_thickness * 2.0, 0.001), 0.0, 1.0)
	return Color(0.25, 0.80, 0.25).lerp(Color(0.28, 0.36, 0.16), maturity)

static func build_target_markers_mesh(
		targets: PackedVector3Array,
		reached_flags: Array[bool],
		acceptance_radius: float,
		wall_normal: Vector3,
		surface_offset: float = 0.025) -> ArrayMesh:
	if targets.is_empty():
		return null

	var st := SurfaceTool.new()
	st.begin(Mesh.PRIMITIVE_LINES)

	var normal := wall_normal.normalized()
	var ref_up := Vector3.UP if absf(normal.dot(Vector3.UP)) < 0.99 else Vector3.RIGHT
	var axis_u := ref_up.cross(normal).normalized()
	var axis_v := normal.cross(axis_u).normalized()

	var segments := 32
	var cross_size := minf(acceptance_radius * 0.35, 0.08)

	for i in range(targets.size()):
		var center := targets[i] + normal * surface_offset
		var is_reached := i < reached_flags.size() and reached_flags[i]
		var color := Color(0.2, 1.0, 0.85) if is_reached else Color(1.0, 0.4, 0.2)

		# Center crosshair
		st.set_color(color); st.add_vertex(center - axis_u * cross_size)
		st.set_color(color); st.add_vertex(center + axis_u * cross_size)
		st.set_color(color); st.add_vertex(center - axis_v * cross_size)
		st.set_color(color); st.add_vertex(center + axis_v * cross_size)

		# Acceptance-radius ring
		for s in range(segments):
			var a0 := TAU * float(s) / float(segments)
			var a1 := TAU * float(s + 1) / float(segments)
			var p0 := center + (axis_u * cos(a0) + axis_v * sin(a0)) * acceptance_radius
			var p1 := center + (axis_u * cos(a1) + axis_v * sin(a1)) * acceptance_radius
			st.set_color(color); st.add_vertex(p0)
			st.set_color(color); st.add_vertex(p1)

	return st.commit()
