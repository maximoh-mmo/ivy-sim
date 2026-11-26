/*
 * Ivy Sim - Procedural ivy generator for Godot 4
 * Copyright (C) 2026 Max Heinze
 *
 * SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
 * Dual-licensed under GNU GPLv3 (see LICENSE) or a commercial license (see COMMERCIAL_LICENSE.md).
 * Distributed WITHOUT ANY WARRANTY; see LICENSE for details.
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
