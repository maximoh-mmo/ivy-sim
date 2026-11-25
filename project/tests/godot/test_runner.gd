# Copyright (c) 2026 Max Heinze. All rights reserved.
#
# Licensed for non-commercial use with attribution only.
# Commercial use is strictly prohibited.

extends SceneTree

const SamplerTests = preload("res://tests/godot/sampler_tests.gd")
const GeneratorTests = preload("res://tests/godot/generator_tests.gd")

func _init() -> void:
	var tests = SamplerTests.new()
	tests.sampler_test()

	var generator_tests = GeneratorTests.new()
	generator_tests.run_all()
	quit()
