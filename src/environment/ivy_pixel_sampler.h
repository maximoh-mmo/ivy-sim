/*
 * Copyright (c) 2026 Max Heinze. All rights reserved.
 *
 * Licensed for non-commercial use with attribution only.
 * Commercial use is strictly prohibited.
 */

#pragma once

#include <vector>

class IvyPixelSampler {
public:
    bool set_pixel_data(
        const std::vector<float>& data,
        int width,
        int height
    );
    float sample_attraction(float u, float v) const;
    bool is_valid() const;

private:
    std::vector<float> m_attraction_data;

    int m_width = 0;
    int m_height = 0;

    float wrap_uv(float value) const;
    float get_pixel(int x, int y) const;

};
