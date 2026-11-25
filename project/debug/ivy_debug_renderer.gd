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
