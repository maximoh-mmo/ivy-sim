/*
 * Copyright (c) 2026 Max Heinze. All rights reserved.
 *
 * Licensed for non-commercial use with attribution only.
 * Commercial use is strictly prohibited.
 */

/**
 * @file ivy_surface_sampler.cpp
 * @brief Implementation of IvySurfaceSampler Godot Image loader and sampler adapter.
 */

#include "ivy_surface_sampler.h"
#include "ivy_pixel_sampler.h"
#include <cstdint>

namespace ivygenerator {
    using namespace godot;

    bool IvySurfaceSampler::load_image(const Ref<Image>& image) {
        if (image.is_null() || image->is_empty()) {
            return false;
        }

        const int width = image->get_width();
        const int height = image->get_height();

        if (width <= 0 || height <= 0) {
            return false;
        }

        const Image::Format format = image->get_format();

        if (format != Image::FORMAT_RGB8 && format != Image::FORMAT_RGBA8 && format != Image::FORMAT_L8) {
            return false;
        }

        const int channels = (format == Image::FORMAT_RGBA8) ? 4 : (format == Image::FORMAT_RGB8) ? 3 : 1;

        const PackedByteArray pixels = image->get_data();

        const int64_t pixel_count = static_cast<int64_t>(width) * height;
    
        const int64_t expected_size = static_cast<int64_t>(width) * height * channels;

        if (pixels.size() != expected_size) {
            return false;
        }

        std::vector<float> data;

        data.reserve(pixel_count);

        for (int64_t i = 0; i < pixel_count; ++i) {
            const uint8_t r = pixels[i * channels];
            if (channels > 1) {
                const uint8_t g = pixels[i * channels + 1];
                const uint8_t b = pixels[i * channels + 2];
                if (r != g || g != b) {
                    return false;
                }
            }
            data.push_back(static_cast<float>(r) / 255.0f);
        }

        return m_pixel_sampler.set_pixel_data(
            data,
            width,
            height
        );
    }

    float IvySurfaceSampler::sample_attraction(const Vector2& uv) const {
        return m_pixel_sampler.sample_attraction(uv.x, uv.y);
    }

    bool IvySurfaceSampler::is_valid() const {
        return m_pixel_sampler.is_valid();
    }
} // namespace ivygenerator