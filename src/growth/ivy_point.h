/*
 * Copyright (c) 2026 Max Heinze. All rights reserved.
 *
 * Licensed for non-commercial use with attribution only.
 * Commercial use is strictly prohibited.
 */

#pragma once

#include <godot_cpp/variant/vector3.hpp>

#include <cstdint>
/**
 * @file ivy_point.h
 * @brief Representation of an individual ivy node/particle in the simulation graph.
 */

namespace ivygenerator {

 struct IvyPoint {
    godot::Vector3 position = godot::Vector3();
    float thickness = 0.0f;
    int32_t parent_index = -1;
};
} // namespace ivygenerator