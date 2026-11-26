# Ivy Sim - Procedural ivy generator for Godot 4
# Copyright (C) 2026 Max Heinze
#
# SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
# Dual-licensed under GNU GPLv3 (see LICENSE) or a commercial license (see COMMERCIAL_LICENSE.md).
# Distributed WITHOUT ANY WARRANTY; see LICENSE for details.

extends RefCounted

var tests_passed := 0
var tests_failed := 0

func sampler_test() -> void:
	tests_passed = 0
	tests_failed = 0

	print("\n=== IvySurfaceSampler Tests ===")

	var sampler = IvySurfaceSamplerTestBridge.new()
	var image = Image.create(2, 2, false, Image.FORMAT_L8)

	# Create a known 2x2 grayscale image.
	image.set_pixel(0, 0, Color(0.0, 0.0, 0.0))
	image.set_pixel(1, 0, Color(64.0 / 255.0, 64.0 / 255.0, 64.0 / 255.0))
	image.set_pixel(0, 1, Color(128.0 / 255.0, 128.0 / 255.0, 128.0 / 255.0))
	image.set_pixel(1, 1, Color(1.0, 1.0, 1.0))

	# Loading tests.
	_run_test("Load image", sampler.load_image(image))
	_run_test("Sampler is valid", sampler.is_valid())

	if not sampler.is_valid():
		print("\nCannot continue: sampler failed to initialize.")
		_print_test_summary()
		return

	# Pixel center tests.
	_run_float_test("Top-left pixel", sampler.sample_attraction(Vector2(0.25, 0.75)), 0.0)
	_run_float_test("Top-right pixel", sampler.sample_attraction(Vector2(0.75, 0.75)), 64.0 / 255.0)
	_run_float_test("Bottom-left pixel", sampler.sample_attraction(Vector2(0.25, 0.25)), 128.0 / 255.0)
	_run_float_test("Bottom-right pixel", sampler.sample_attraction(Vector2(0.75, 0.25)), 1.0)

	# Bilinear interpolation test.
	var expected_center := (64.0 + 128.0 + 255.0) / (4.0 * 255.0)
	_run_float_test("Bilinear center", sampler.sample_attraction(Vector2(0.5, 0.5)), expected_center)

	# Horizontal wrapping tests.
	_run_float_test("Positive U wrapping", sampler.sample_attraction(Vector2(1.75, 0.75)), 64.0 / 255.0)
	_run_float_test("Negative U wrapping", sampler.sample_attraction(Vector2(-0.25, 0.75)), 64.0 / 255.0)

	# Vertical wrapping tests.
	_run_float_test("Positive V wrapping", sampler.sample_attraction(Vector2(0.75, 1.75)), 64.0 / 255.0)
	_run_float_test("Negative V wrapping", sampler.sample_attraction(Vector2(0.75, -0.25)), 64.0 / 255.0)

	test_failed_load_preserves_data()
	test_rgb_and_rgba_formats()
	test_reject_non_grayscale_images()
	test_single_pixel_image()

	_run_float_test(
		"UV (0,0) wrapping",
		sampler.sample_attraction(Vector2(0.0, 0.0)),
		sampler.sample_attraction(Vector2(1.0, 1.0))
	)

	_run_float_test(
		"Multi-period U/V wrapping",
		sampler.sample_attraction(Vector2(-2.25, 3.75)),
		sampler.sample_attraction(Vector2(0.75, 0.75))
	)
	
	test_invalid_sampler()
	
	_print_test_summary()


func _run_test(test_name: String, result: bool) -> void:
	if result:
		tests_passed += 1
		print("PASS: ", test_name)
	else:
		tests_failed += 1
		print("FAIL: ", test_name)


func _run_float_test(test_name: String, actual: float, expected: float, tolerance: float = 0.00001) -> void:
	var passed := absf(actual - expected) <= tolerance

	if passed:
		tests_passed += 1
		print("PASS: ", test_name)
	else:
		tests_failed += 1
		print("FAIL: ", test_name,
			" | Expected: ", expected,
			" | Actual: ", actual)


func _print_test_summary() -> void:
	print("\n=== Test Summary ===")
	print("Passed: ", tests_passed)
	print("Failed: ", tests_failed)
	print("====================\n")

func test_failed_load_preserves_data() -> void:
	var sampler = IvySurfaceSamplerTestBridge.new()

	# Load a valid image.
	var valid_image = Image.create(2, 2, false, Image.FORMAT_L8)
	valid_image.fill(Color(0.5, 0.5, 0.5))

	var loaded = sampler.load_image(valid_image)
	_run_test("Initial valid image loads", loaded)

	if not loaded:
		return

	# Record a sample before attempting the invalid load.
	var original_value = sampler.sample_attraction(Vector2(0.5, 0.5))

	# Attempt to load an invalid image.
	var invalid_image = Image.new()
	var invalid_loaded = sampler.load_image(invalid_image)

	_run_test("Invalid image is rejected", not invalid_loaded)

	# Check that the original data is still available.
	var new_value = sampler.sample_attraction(Vector2(0.5, 0.5))

	_run_float_test(
		"Previous image data is preserved",
		new_value,
		original_value
	)

	_run_test("Sampler remains valid", sampler.is_valid())

func test_rgb_and_rgba_formats() -> void:
	var sampler = IvySurfaceSamplerTestBridge.new()

	# Test RGB8.
	var rgb_image = Image.create(2, 2, false, Image.FORMAT_RGB8)
	rgb_image.fill(Color(0.5, 0.5, 0.5))

	var rgb_loaded = sampler.load_image(rgb_image)
	_run_test("RGB8 grayscale image loads", rgb_loaded)

	if rgb_loaded:
		_run_float_test(
			"RGB8 grayscale sampling",
			sampler.sample_attraction(Vector2(0.5, 0.5)),
			0.5,
			1.0 / 255.0
		)

	# Test RGBA8.
	var rgba_image = Image.create(2, 2, false, Image.FORMAT_RGBA8)
	rgba_image.fill(Color(0.5, 0.5, 0.5, 1.0))

	var rgba_loaded = sampler.load_image(rgba_image)
	_run_test("RGBA8 grayscale image loads", rgba_loaded)

	if rgba_loaded:
		_run_float_test(
			"RGBA8 grayscale sampling",
			sampler.sample_attraction(Vector2(0.5, 0.5)),
			0.5,
			1.0 / 255.0
		)

func test_reject_non_grayscale_images() -> void:
	var sampler = IvySurfaceSamplerTestBridge.new()

	# Establish valid image data first.
	var valid_image = Image.create(2, 2, false, Image.FORMAT_L8)
	valid_image.fill(Color(0.5, 0.5, 0.5))

	_run_test("Initial valid image loads", sampler.load_image(valid_image))

	var original_value = sampler.sample_attraction(Vector2(0.5, 0.5))

	# RGB8 with unequal channels.
	var rgb_image = Image.create(2, 2, false, Image.FORMAT_RGB8)
	rgb_image.fill(Color(1.0, 0.0, 0.0))

	_run_test("Non-grayscale RGB8 rejected", not sampler.load_image(rgb_image))

	_run_float_test(
		"RGB8 failure preserves previous data",
		sampler.sample_attraction(Vector2(0.5, 0.5)),
		original_value
	)

	# RGBA8 with unequal channels.
	var rgba_image = Image.create(2, 2, false, Image.FORMAT_RGBA8)
	rgba_image.fill(Color(0.0, 1.0, 0.0, 1.0))

	_run_test("Non-grayscale RGBA8 rejected", not sampler.load_image(rgba_image))

	_run_float_test(
		"RGBA8 failure preserves previous data",
		sampler.sample_attraction(Vector2(0.5, 0.5)),
		original_value
	)

	_run_test("Sampler remains valid", sampler.is_valid())

func test_single_pixel_image() -> void:
	var sampler = IvySurfaceSamplerTestBridge.new()

	var image = Image.create(1, 1, false, Image.FORMAT_L8)
	image.set_pixel(0, 0, Color(128.0 / 255.0, 128.0 / 255.0, 128.0 / 255.0))

	_run_test("1x1 image loads", sampler.load_image(image))
	_run_test("1x1 sampler is valid", sampler.is_valid())

	# The only pixel should be returned for every UV coordinate.
	var expected := 128.0 / 255.0

	_run_float_test(
		"1x1 center sampling",
		sampler.sample_attraction(Vector2(0.5, 0.5)),
		expected
	)

	_run_float_test(
		"1x1 wrapped sampling",
		sampler.sample_attraction(Vector2(1.75, -2.25)),
		expected
	)

	_run_float_test(
		"1x1 boundary sampling",
		sampler.sample_attraction(Vector2(0.0, 0.0)),
		expected
	)

func test_invalid_sampler() -> void:
	var sampler = IvySurfaceSamplerTestBridge.new()

	_run_test("New sampler is invalid", not sampler.is_valid())

	_run_float_test(
		"Invalid sampler returns zero",
		sampler.sample_attraction(Vector2(0.5, 0.5)),
		0.0
	)