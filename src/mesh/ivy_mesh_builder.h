/*
 * Copyright (c) 2026 Max Heinze. All rights reserved.
 *
 * Licensed for non-commercial use with attribution only.
 * Commercial use is strictly prohibited.
 */

#pragma once

#include "ivy_mesh_settings.h"

#include <godot_cpp/classes/array_mesh.hpp>
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/variant/packed_float32_array.hpp>
#include <godot_cpp/variant/packed_int32_array.hpp>
#include <godot_cpp/variant/packed_vector3_array.hpp>
#include <godot_cpp/variant/transform3d.hpp>
#include <godot_cpp/variant/typed_array.hpp>
#include <godot_cpp/variant/vector3.hpp>

namespace ivygenerator {

class IvyMeshBuilder : public godot::RefCounted {
	GDCLASS(IvyMeshBuilder, godot::RefCounted)

protected:
	static void _bind_methods();

public:
	IvyMeshBuilder() = default;
	~IvyMeshBuilder() override = default;

	void set_settings(const godot::Ref<IvyMeshSettings> &settings);
	godot::Ref<IvyMeshSettings> get_settings() const;

	godot::Ref<godot::ArrayMesh> build_stem_mesh(
			const godot::PackedVector3Array &positions,
			const godot::PackedFloat32Array &thicknesses,
			const godot::PackedInt32Array &parent_indices,
			const godot::Vector3 &wall_normal) const;

	godot::TypedArray<godot::Transform3D> build_leaf_transforms(
			const godot::PackedVector3Array &positions,
			const godot::PackedFloat32Array &thicknesses,
			const godot::PackedInt32Array &parent_indices,
			const godot::Vector3 &wall_normal) const;

private:
	godot::Ref<IvyMeshSettings> m_settings;
};

} // namespace ivygenerator