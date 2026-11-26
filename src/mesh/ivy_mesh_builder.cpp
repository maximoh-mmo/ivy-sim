/*
 * Copyright (c) 2026 Max Heinze. All rights reserved.
 *
 * Licensed for non-commercial use with attribution only.
 * Commercial use is strictly prohibited.
 */

#include "ivy_mesh_builder.h"

#include "ivy_mesh_data.h"
#include "ivy_mesh_generator.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/packed_color_array.hpp>
#include <godot_cpp/variant/packed_vector2_array.hpp>

#include <vector>

namespace ivygenerator {
using namespace godot;

namespace {

void unpack_graph_arrays(
		const PackedVector3Array &positions,
		const PackedFloat32Array &thicknesses,
		const PackedInt32Array &parent_indices,
		std::vector<Vector3> &out_positions,
		std::vector<float> &out_thicknesses,
		std::vector<int32_t> &out_parents) {
	const int64_t pos_count = positions.size();
	const int64_t thick_count = thicknesses.size();
	const int64_t parent_count = parent_indices.size();

	out_positions.resize(static_cast<size_t>(pos_count));
	for (int64_t i = 0; i < pos_count; ++i) {
		out_positions[static_cast<size_t>(i)] = positions[i];
	}

	out_thicknesses.resize(static_cast<size_t>(thick_count));
	for (int64_t i = 0; i < thick_count; ++i) {
		out_thicknesses[static_cast<size_t>(i)] = thicknesses[i];
	}

	out_parents.resize(static_cast<size_t>(parent_count));
	for (int64_t i = 0; i < parent_count; ++i) {
		out_parents[static_cast<size_t>(i)] = parent_indices[i];
	}
}

} // namespace

void IvyMeshBuilder::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_settings", "settings"), &IvyMeshBuilder::set_settings);
	ClassDB::bind_method(D_METHOD("get_settings"), &IvyMeshBuilder::get_settings);
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "settings", PROPERTY_HINT_RESOURCE_TYPE, "IvyMeshSettings"),
			"set_settings", "get_settings");

	ClassDB::bind_method(
			D_METHOD("build_stem_mesh", "positions", "thicknesses", "parent_indices", "wall_normal"),
			&IvyMeshBuilder::build_stem_mesh);

	ClassDB::bind_method(
			D_METHOD("build_leaf_transforms", "positions", "thicknesses", "parent_indices", "wall_normal"),
			&IvyMeshBuilder::build_leaf_transforms);
}

void IvyMeshBuilder::set_settings(const Ref<IvyMeshSettings> &settings) {
	m_settings = settings;
}

Ref<IvyMeshSettings> IvyMeshBuilder::get_settings() const {
	return m_settings;
}

Ref<ArrayMesh> IvyMeshBuilder::build_stem_mesh(
		const PackedVector3Array &positions,
		const PackedFloat32Array &thicknesses,
		const PackedInt32Array &parent_indices,
		const Vector3 &wall_normal) const {
	if (m_settings.is_null()) {
		return Ref<ArrayMesh>();
	}

	std::vector<Vector3> native_positions;
	std::vector<float> native_thicknesses;
	std::vector<int32_t> native_parents;
	unpack_graph_arrays(positions, thicknesses, parent_indices,
			native_positions, native_thicknesses, native_parents);

	IvyMeshData mesh_data;
	const IvyMeshConfig config = m_settings->to_mesh_config();
	if (!IvyMeshGenerator::build_stem_mesh(
				native_positions, native_thicknesses, native_parents, wall_normal, config, mesh_data)) {
		return Ref<ArrayMesh>();
	}

	Ref<ArrayMesh> mesh;
	mesh.instantiate();

	if (mesh_data.is_empty()) {
		return mesh;
	}

	const int64_t vcount = static_cast<int64_t>(mesh_data.vertices.size());
	const int64_t icount = static_cast<int64_t>(mesh_data.indices.size());

	PackedVector3Array out_verts;
	PackedVector3Array out_normals;
	PackedVector2Array out_uvs;
	PackedColorArray out_colors;
	PackedInt32Array out_indices;

	out_verts.resize(vcount);
	out_normals.resize(vcount);
	out_uvs.resize(vcount);
	out_colors.resize(vcount);
	out_indices.resize(icount);

	Vector3 *verts_w = out_verts.ptrw();
	Vector3 *normals_w = out_normals.ptrw();
	Vector2 *uvs_w = out_uvs.ptrw();
	Color *colors_w = out_colors.ptrw();
	int32_t *indices_w = out_indices.ptrw();

	for (int64_t i = 0; i < vcount; ++i) {
		const size_t idx = static_cast<size_t>(i);
		verts_w[i] = mesh_data.vertices[idx];
		normals_w[i] = mesh_data.normals[idx];
		uvs_w[i] = mesh_data.uvs[idx];
		colors_w[i] = mesh_data.colors[idx];
	}
	for (int64_t i = 0; i < icount; ++i) {
		indices_w[i] = mesh_data.indices[static_cast<size_t>(i)];
	}

	Array arrays;
	arrays.resize(Mesh::ARRAY_MAX);
	arrays[Mesh::ARRAY_VERTEX] = out_verts;
	arrays[Mesh::ARRAY_NORMAL] = out_normals;
	arrays[Mesh::ARRAY_TEX_UV] = out_uvs;
	arrays[Mesh::ARRAY_COLOR] = out_colors;
	arrays[Mesh::ARRAY_INDEX] = out_indices;

	mesh->add_surface_from_arrays(Mesh::PRIMITIVE_TRIANGLES, arrays);
	return mesh;
}

TypedArray<Transform3D> IvyMeshBuilder::build_leaf_transforms(
		const PackedVector3Array &positions,
		const PackedFloat32Array &thicknesses,
		const PackedInt32Array &parent_indices,
		const Vector3 &wall_normal) const {
	TypedArray<Transform3D> result;
	if (m_settings.is_null()) {
		return result;
	}

	std::vector<Vector3> native_positions;
	std::vector<float> native_thicknesses;
	std::vector<int32_t> native_parents;
	unpack_graph_arrays(positions, thicknesses, parent_indices,
			native_positions, native_thicknesses, native_parents);

	std::vector<Transform3D> native_transforms;
	const IvyMeshConfig config = m_settings->to_mesh_config();
	if (!IvyMeshGenerator::build_leaf_transforms(
				native_positions, native_thicknesses, native_parents, wall_normal, config, native_transforms)) {
		return result;
	}

	const int64_t count = static_cast<int64_t>(native_transforms.size());
	result.resize(count);
	for (int64_t i = 0; i < count; ++i) {
		result[i] = native_transforms[static_cast<size_t>(i)];
	}

	return result;
}

} // namespace ivygenerator