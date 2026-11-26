# Ivy Sim - Procedural ivy generator for Godot 4
# Copyright (C) 2026 Max Heinze
#
# SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
# Dual-licensed under GNU GPLv3 (see LICENSE) or a commercial license (see COMMERCIAL_LICENSE.md).
# Distributed WITHOUT ANY WARRANTY; see LICENSE for details.

extends SceneTree

const SamplerTests = preload("res://tests/godot/sampler_tests.gd")
const GeneratorTests = preload("res://tests/godot/generator_tests.gd")
const BuilderTests = preload("res://tests/godot/mesh_builder_tests.gd")

func _init() -> void:
	var tests = SamplerTests.new()
	tests.sampler_test()

	var generator_tests = GeneratorTests.new()
	generator_tests.run_all()

	var builder_tests = BuilderTests.new()
	builder_tests.run_all()

	quit()
