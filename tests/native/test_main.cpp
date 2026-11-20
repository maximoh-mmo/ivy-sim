/*
 * Copyright (c) 2026 Max Heinze. All rights reserved.
 *
 * Licensed for non-commercial use with attribution only.
 * Commercial use is strictly prohibited.
 */

/**
 * @file test_main.cpp
 * @brief Standalone test runner entry point for all ivy-sim C++ unit tests.
 */

#include "ivy_random_tests.h"
#include "ivy_wall_tests.h"
#include "ivy_pixel_sampler_tests.h"
#include "ivy_growth_graph_tests.h"

using namespace ivygenerator;

/**
 * @brief Main test runner function executing IvyRandom, IvyWall, and IvyPixelSampler test suites.
 * @return int 0 if all test suites pass, 1 if any failure occurs.
 */
int main() {
    IvyRandomTests random_tests;
    IvyWallTests wall_tests;
    IvyPixelSamplerTests pixel_sampler_tests;
    IvyGrowthGraphTests graph_tests;
    const bool random_passed = random_tests.run_all();
    const bool wall_passed = wall_tests.run_all();
    const bool pixel_sampler_passed = pixel_sampler_tests.run_all();
    const bool ivy_growth_graph_passed = graph_tests.run_all();

    return random_passed && wall_passed && pixel_sampler_passed && ivy_growth_graph_passed ? 0 : 1;
}
