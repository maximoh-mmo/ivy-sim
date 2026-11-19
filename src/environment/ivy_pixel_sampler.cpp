#include "ivy_pixel_sampler.h"
#include <cmath>

float IvyPixelSampler::wrap_uv(float value) const {
    return value - std::floor(value);
}

float IvyPixelSampler::get_pixel(int x, int y) const {
    x = (x + m_width) % m_width;
    y = (y + m_height) % m_height;

    return m_attraction_data[
        static_cast<size_t>(y) * static_cast<size_t>(m_width) +
        static_cast<size_t>(x)
    ];
}

float IvyPixelSampler::sample_attraction(float u, float v) const {
    if (!is_valid()) {
        return 0.0f;
    }

    u = wrap_uv(u);
    v = wrap_uv(v);

    const float x = u * static_cast<float>(m_width) - 0.5f;
    const float y = (1.0f - v) * static_cast<float>(m_height) - 0.5f;

    const int x0 = static_cast<int>(std::floor(x));
    const int y0 = static_cast<int>(std::floor(y));

    const float tx = x - static_cast<float>(x0);
    const float ty = y - static_cast<float>(y0);

    const float top_left = get_pixel(x0, y0);
    const float top_right = get_pixel(x0 + 1, y0);
    const float bottom_left = get_pixel(x0, y0 + 1);
    const float bottom_right = get_pixel(x0 + 1, y0 + 1);

    const float top = top_left + (top_right - top_left) * tx;
    const float bottom = bottom_left + (bottom_right - bottom_left) * tx;

    return top + (bottom - top) * ty;
}

bool IvyPixelSampler::set_pixel_data(
    const std::vector<float>& data,
    int width,
    int height
) {
    if (width <= 0 || height <= 0) {
        return false;
    }

    const size_t expected_size =
        static_cast<size_t>(width) * static_cast<size_t>(height);

    if (data.size() != expected_size) {
        return false;
    }

    m_attraction_data = data;
    m_width = width;
    m_height = height;

    return true;
}

bool IvyPixelSampler::is_valid() const {
    return m_width > 0 &&
           m_height > 0 &&
           m_attraction_data.size() ==
               static_cast<size_t>(m_width) * static_cast<size_t>(m_height);
}