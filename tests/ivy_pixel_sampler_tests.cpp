/*
 * Copyright (c) 2026 Max Heinze. All rights reserved.
 *
 * Licensed for non-commercial use with attribution only.
 * Commercial use is strictly prohibited.
 */

#include "ivy_pixel_sampler_tests.h"
#include "../src/environment/ivy_pixel_sampler.h"

#include <iostream>
#include <cmath>

bool IvyPixelSamplerTests::run_test(const char* name, bool (IvyPixelSamplerTests::*test)()) {
    std::cout << "Running: " << name << '\n';

    const bool result = (this->*test)();

    if (result) {
        ++m_passed;
        std::cout << "    PASS\n";
    } else {
        ++m_failed;
        std::cout << "    FAIL\n";
    }

    return result;
}

bool IvyPixelSamplerTests::expect(
		bool condition,
		const char *message) {
	if (!condition) {
        std::cout << "FAILED: " << message << '\n';
    }

    return condition;
}

bool IvyPixelSamplerTests::test_initial_state() {
    IvyPixelSampler sampler;

    return expect(!sampler.is_valid(),
        "New sampler should be invalid");
}

bool IvyPixelSamplerTests::test_set_pixel_data() {
    IvyPixelSampler sampler;

    const std::vector<float> pixels = {
        0.0f, 1.0f,
        0.25f, 0.75f
    };

    const bool result = sampler.set_pixel_data(pixels, 2, 2);

    return expect(result && sampler.is_valid(),
        "Valid pixel data should be accepted");
}

bool IvyPixelSamplerTests::test_reject_invalid_dimensions() {
    IvyPixelSampler sampler;

    const std::vector<float> pixels = { 0.5f };

    return expect(!sampler.set_pixel_data(pixels, 0, 1),
        "Zero width should be rejected");
}

bool IvyPixelSamplerTests::test_reject_incorrect_data_size() {
    IvyPixelSampler sampler;

    const std::vector<float> pixels = {
        0.0f, 0.5f, 1.0f
    };

    return expect(!sampler.set_pixel_data(pixels, 2, 2),
        "Incorrect pixel count should be rejected");
}

bool IvyPixelSamplerTests::test_failed_update_preserves_data() {
    IvyPixelSampler sampler;

    const std::vector<float> valid_pixels = { 0.25f };
    sampler.set_pixel_data(valid_pixels, 1, 1);

    const std::vector<float> invalid_pixels = { 0.5f, 0.75f };
    const bool result = sampler.set_pixel_data(invalid_pixels, 2, 2);

    return expect(!result && sampler.is_valid(),
        "Failed update should preserve previous valid data");
}

bool IvyPixelSamplerTests::test_sample_invalid_returns_zero() {
    IvyPixelSampler sampler;
    return expect(sampler.sample_attraction(0.5f, 0.5f) == 0.0f,
        "Sampling an invalid sampler should return 0.0");
}

bool IvyPixelSamplerTests::test_sample_bilinear() {
    IvyPixelSampler sampler;
    const std::vector<float> pixels = {
        1.0f, 1.0f,
        1.0f, 1.0f
    };
    sampler.set_pixel_data(pixels, 2, 2);

    const float sample = sampler.sample_attraction(0.5f, 0.5f);
    return expect(std::abs(sample - 1.0f) < 0.001f,
        "Sampling uniform grid should return 1.0");
}

bool IvyPixelSamplerTests::test_sample_uv_wrapping() {
    IvyPixelSampler sampler;
    const std::vector<float> pixels = {
        0.2f, 0.8f,
        0.4f, 0.6f
    };
    sampler.set_pixel_data(pixels, 2, 2);

    const float base = sampler.sample_attraction(0.3f, 0.7f);
    const float wrapped = sampler.sample_attraction(2.3f, -1.3f);

    return expect(std::abs(base - wrapped) < 0.0001f,
        "Wrapped UV sampling should match base UV sampling");
}

bool IvyPixelSamplerTests::run_all() {
    m_passed = 0;
    m_failed = 0;

    std::cout << "\n=== IvyPixelSampler Unit Tests ===\n\n";

    run_test("Initial state",
        &IvyPixelSamplerTests::test_initial_state);

    run_test("Set pixel data",
        &IvyPixelSamplerTests::test_set_pixel_data);

    run_test("Reject invalid dimensions",
        &IvyPixelSamplerTests::test_reject_invalid_dimensions);

    run_test("Reject incorrect data size",
        &IvyPixelSamplerTests::test_reject_incorrect_data_size);

    run_test("Failed update preserves data",
        &IvyPixelSamplerTests::test_failed_update_preserves_data);

    run_test("Sample invalid returns zero",
        &IvyPixelSamplerTests::test_sample_invalid_returns_zero);

    run_test("Sample bilinear interpolation",
        &IvyPixelSamplerTests::test_sample_bilinear);

    run_test("Sample UV wrapping",
        &IvyPixelSamplerTests::test_sample_uv_wrapping);

    std::cout << "\n=== Test Summary ===\n";
    std::cout << "Passed: " << m_passed << '\n';
    std::cout << "Failed: " << m_failed << '\n';

    return m_failed == 0;
}
