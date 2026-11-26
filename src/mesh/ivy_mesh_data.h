#pragma once

#include <godot_cpp/variant/color.hpp>
#include <godot_cpp/variant/vector2.hpp>
#include <godot_cpp/variant/vector3.hpp>

#include <cstdint>
#include <vector>

namespace ivygenerator {

struct IvyMeshData {
	std::vector<godot::Vector3> vertices;
	std::vector<godot::Vector3> normals;
	std::vector<godot::Vector2> uvs;
	std::vector<godot::Color> colors;
	std::vector<int32_t> indices;

	void clear() {
		vertices.clear();
		normals.clear();
		uvs.clear();
		colors.clear();
		indices.clear();
	}

	bool is_empty() const {
		return vertices.empty() || indices.empty();
	}
};

} // namespace ivygenerator