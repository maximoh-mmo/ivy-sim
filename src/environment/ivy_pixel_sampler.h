/*
 * Copyright (c) 2026 Max Heinze. All rights reserved.
 *
 * Licensed for non-commercial use with attribution only.
 * Commercial use is strictly prohibited.
 */

#pragma once

#include <vector>

/**
 * @file ivy_pixel_sampler.h
 * @brief Standalone bilinear texture sampler for attraction mapping.
 */

namespace ivygenerator {

/**
 * @class IvyPixelSampler
 * @brief Independent 2D scalar field sampler supporting bilinear interpolation and UV wrapping.
 *
 * Designed without Godot runtime dependencies so it can be compiled into both
 * the GDExtension runtime and standalone C++ unit test runners.
 */
class IvyPixelSampler {
public:
    /**
     * @brief Loads a row-major 1D array of scalar pixel float data.
     * @param data Vector of scalar float values in [0.0, 1.0].
     * @param width Image width in pixels (> 0).
     * @param height Image height in pixels (> 0).
     * @return bool True if dimensions match data size and data was updated.
     */
    bool set_pixel_data(
        const std::vector<float>& data,
        int width,
        int height
    );

    /**
     * @brief Samples the attraction map at continuous normalized coordinates (u, v) using bilinear filtering.
     *
     * Automatically wraps UV coordinates to [0.0, 1.0) and flips vertical axis to match standard image orientation.
     *
     * @param u Horizontal texture coordinate.
     * @param v Vertical texture coordinate.
     * @return float Interpolated attraction scalar, or 0.0f if sampler is invalid.
     */
    float sample_attraction(float u, float v) const;

    /**
     * @brief Checks if the sampler contains valid, correctly-sized pixel buffer data.
     * @return bool True if buffer is non-empty and matches width * height.
     */
    bool is_valid() const;

private:
    std::vector<float> m_attraction_data; /**< Row-major buffer of scalar pixel values. */

    int m_width = 0;   /**< Texture width in pixels. */
    int m_height = 0;  /**< Texture height in pixels. */

    /**
     * @brief Wraps arbitrary floating point texture coordinates into the range [0.0, 1.0).
     * @param value Raw coordinate value.
     * @return float Wrapped coordinate in [0.0, 1.0).
     */
    float wrap_uv(float value) const;

    /**
     * @brief Fetches a pixel from buffer with toroidal coordinate wrapping.
     * @param x Integer horizontal pixel index.
     * @param y Integer vertical pixel index.
     * @return float Scalar value at wrapped coordinate (x, y).
     */
    float get_pixel(int x, int y) const;
};

} // namespace ivygenerator
