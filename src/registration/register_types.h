/*
 * Copyright (c) 2026 Max Heinze. All rights reserved.
 *
 * Licensed for non-commercial use with attribution only.
 * Commercial use is strictly prohibited.
 */

#pragma once

#include <godot_cpp/core/class_db.hpp>

/**
 * @file register_types.h
 * @brief GDExtension initialization and termination declarations.
 */

using namespace godot;

/**
 * @brief Initializes GDExtension classes and types for the given engine module level.
 * @param p_level Godot initialization level (core, servers, scene, editor).
 */
void initialize_gdextension_types(ModuleInitializationLevel p_level);

/**
 * @brief Cleans up and uninitializes GDExtension classes at termination.
 * @param p_level Godot initialization level.
 */
void uninitialize_gdextension_types(ModuleInitializationLevel p_level);
