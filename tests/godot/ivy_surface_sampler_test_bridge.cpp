/*
 * Ivy Sim - Procedural ivy generator for Godot 4
 * Copyright (C) 2026 Max Heinze
 *
 * SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
 * Dual-licensed under GNU GPLv3 (see LICENSE) or a commercial license (see COMMERCIAL_LICENSE.md).
 * Distributed WITHOUT ANY WARRANTY; see LICENSE for details.
 */

/**
 * @file ivy_surface_sampler_test_bridge.cpp
 * @brief Implementation of IvySurfaceSamplerTestBridge ClassDB bindings and wrapper calls.
 */

#include "ivy_surface_sampler_test_bridge.h"

namespace ivygenerator {

using namespace godot;

void IvySurfaceSamplerTestBridge::_bind_methods() {
	ClassDB::bind_method(
			D_METHOD("load_image", "image"),
			&IvySurfaceSamplerTestBridge::load_image);

	ClassDB::bind_method(
			D_METHOD("sample_attraction", "uv"),
			&IvySurfaceSamplerTestBridge::sample_attraction);

	ClassDB::bind_method(
			D_METHOD("is_valid"),
			&IvySurfaceSamplerTestBridge::is_valid);
}

bool IvySurfaceSamplerTestBridge::load_image(const Ref<Image> &image) {
	return m_sampler.load_image(image);
}

float IvySurfaceSamplerTestBridge::sample_attraction(const Vector2 &uv) const {
	return m_sampler.sample_attraction(uv);
}

bool IvySurfaceSamplerTestBridge::is_valid() const {
	return m_sampler.is_valid();
}

} // namespace ivygenerator
