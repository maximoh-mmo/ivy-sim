/*
 * Copyright (c) 2026 Max Heinze. All rights reserved.
 *
 * Licensed for non-commercial use with attribution only.
 * Commercial use is strictly prohibited.
 */

#include "register_types.h"

#include <gdextension_interface.h>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/godot.hpp>

#include "godot/ivy_surface_sampler_test_bridge.h"
#include "ivy_generator/ivy_generator.h"

/**
 * @file register_types.cpp
 * @brief GDExtension entry point and class registration implementation.
 */

using namespace godot;

void initialize_gdextension_types(ModuleInitializationLevel p_level) {
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
		return;
	}
	GDREGISTER_CLASS(ivygenerator::IvyGenerator);
	GDREGISTER_CLASS(ivygenerator::IvySurfaceSamplerTestBridge);
}

void uninitialize_gdextension_types(ModuleInitializationLevel p_level) {
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
		return;
	}
}

extern "C" {
/**
 * @brief Main entry point symbol called by the Godot engine when loading the GDExtension.
 * @param p_get_proc_address Interface function pointer lookup.
 * @param p_library Pointer to the library instance.
 * @param r_initialization Output initialization struct to populate.
 * @return GDExtensionBool True on successful initialization.
 */
GDExtensionBool GDE_EXPORT ivysim(
		GDExtensionInterfaceGetProcAddress p_get_proc_address,
		GDExtensionClassLibraryPtr p_library,
		GDExtensionInitialization *r_initialization) {
	GDExtensionBinding::InitObject init_obj(p_get_proc_address, p_library, r_initialization);
	init_obj.register_initializer(initialize_gdextension_types);
	init_obj.register_terminator(uninitialize_gdextension_types);
	init_obj.set_minimum_library_initialization_level(MODULE_INITIALIZATION_LEVEL_SCENE);

	return init_obj.init();
}
}
