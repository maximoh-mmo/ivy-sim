/*
 * Copyright (c) 2026 Max Heinze. All rights reserved.
 *
 * Licensed for non-commercial use with attribution only.
 * Commercial use is strictly prohibited.
 */

#pragma once
#include <godot_cpp/classes/image.hpp>
#include "ivy_pixel_sampler.h"

using namespace godot;

class IvySurfaceSampler {
public:
    bool load_image(const Ref<Image>& image);

    float sample_attraction(const Vector2& uv) const;

    bool is_valid() const;

private:
    IvyPixelSampler m_pixel_sampler;
};