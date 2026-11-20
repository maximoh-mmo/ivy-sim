/*
 * Copyright (c) 2026 Max Heinze. All rights reserved.
 *
 * Licensed for non-commercial use with attribution only.
 * Commercial use is strictly prohibited.
 */

#pragma once

/**
 * @file ivy_pixel_sampler_tests.h
 * @brief Unit test suite for IvyPixelSampler.
 */

/**
 * @class IvyPixelSamplerTests
 * @brief Test suite verifying bilinear interpolation, dimension validation, and UV wrapping.
 */
namespace ivygenerator {
    class IvyPixelSamplerTests {
    public:
        /**
         * @brief Executes all IvyPixelSampler unit tests and prints results to stdout.
         * @return bool True if all tests passed.
         */
        bool run_all();

    private:
        /**
         * @brief Helper to execute and format an individual member test function.
         * @param name Test display name.
         * @param test Member function pointer to the test method.
         * @return bool True if test succeeded.
         */
        bool run_test(const char* name, bool (IvyPixelSamplerTests::*test)());

        /**
         * @brief Assertion helper tracking passed and failed test count.
         * @param condition Condition expression to evaluate.
         * @param message Failure explanation printed when condition is false.
         * @return bool True if condition was true.
         */
        bool expect(bool condition, const char* message);

        bool test_initial_state();
        bool test_set_pixel_data();
        bool test_reject_invalid_dimensions();
        bool test_reject_incorrect_data_size();
        bool test_failed_update_preserves_data();
        bool test_sample_invalid_returns_zero();
        bool test_sample_bilinear();
        bool test_sample_uv_wrapping();

        int m_passed = 0; /**< Count of passing assertions. */
        int m_failed = 0; /**< Count of failing assertions. */
    };
} // namespace ivygenerator