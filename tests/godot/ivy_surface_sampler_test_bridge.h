/*
 * Ivy Sim - Procedural ivy generator for Godot 4
 * Copyright (C) 2026 Max Heinze
 *
 * SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
 * Dual-licensed under GNU GPLv3 (see LICENSE) or a commercial license (see COMMERCIAL_LICENSE.md).
 * Distributed WITHOUT ANY WARRANTY; see LICENSE for details.
 */

#pragma once

#include <godot_cpp/classes/image.hpp>
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/variant/vector2.hpp>

#include "environment/ivy_surface_sampler.h"

/**
 * @file ivy_surface_sampler_test_bridge.h
 * @brief GDExtension bridge exposing IvySurfaceSampler to Godot GDScript integration tests.
 */

namespace ivygenerator {

/**
 * @class IvySurfaceSamplerTestBridge
 * @brief Godot RefCounted bridge exposing IvySurfaceSampler methods to GDScript.
 */
class IvySurfaceSamplerTestBridge : public godot::RefCounted {
	GDCLASS(IvySurfaceSamplerTestBridge, godot::RefCounted)

private:
	IvySurfaceSampler m_sampler; /**< Wrapped native surface sampler instance. */

protected:
	static void _bind_methods();

public:
	/**
	 * @brief Loads and parses a Godot Image for attraction sampling.
	 * @param image Ref<Image> to load.
	 * @return bool True on successful load.
	 */
	bool load_image(const godot::Ref<godot::Image> &image);

	/**
	 * @brief Samples the attraction map at given UV coordinate.
	 * @param uv UV coordinate vector.
	 * @return float Interpolated scalar attraction value.
	 */
	float sample_attraction(const godot::Vector2 &uv) const;

	/**
	 * @brief Checks if the underlying sampler contains valid data.
	 * @return bool True if valid.
	 */
	bool is_valid() const;
};

} // namespace ivygenerator
