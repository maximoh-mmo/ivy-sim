/*
 * Copyright (c) 2026 Max Heinze. All rights reserved.
 *
 * Licensed for non-commercial use with attribution only.
 * Commercial use is strictly prohibited.
 */

#pragma once

#include "ivy_pixel_sampler.h"
#include <godot_cpp/classes/image.hpp>

/**
 * @file ivy_surface_sampler.h
 * @brief Godot Image adapter for surface attraction sampling.
 */

/**
 * @class IvySurfaceSampler
 * @brief Godot-aware wrapper for loading engine Image resources into IvyPixelSampler.
 *
 * Supports grayscale (L8), RGB8, and RGBA8 formats, validating that multi-channel
 * images represent uniform grayscale attraction values.
 */

namespace ivygenerator {

class IvySurfaceSampler {
public:
	/**
	 * @brief Parses a Godot Image resource into floating-point scalar attraction data.
	 * @param image Ref<Image> pointing to a valid Godot Image.
	 * @return bool True if the image was successfully validated and loaded.
	 */
	bool load_image(const godot::Ref<godot::Image> &image);

	/**
	 * @brief Samples the underlying pixel sampler using a Godot Vector2 UV coordinate.
	 * @param uv UV coordinate vector.
	 * @return float Interpolated attraction scalar.
	 */
	float sample_attraction(const godot::Vector2 &uv) const;

	/**
	 * @brief Checks if the underlying pixel sampler contains valid data.
	 * @return bool True if ready for sampling.
	 */
	bool is_valid() const;

	/**
	 * @brief Loads raw scalar float pixel data directly into the underlying sampler.
	 * @param data Vector of scalar float values in [0.0, 1.0].
	 * @param width Image width in pixels (> 0).
	 * @param height Image height in pixels (> 0).
	 * @return bool True if data was accepted.
	 */
	bool set_pixel_data(
			const std::vector<float> &data,
			int width,
			int height);

private:
	IvyPixelSampler m_pixel_sampler; /**< Internal standalone pixel sampler instance. */
};
} // namespace ivygenerator