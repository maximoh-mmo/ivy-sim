# Copyright (c) 2026 Max Heinze. All rights reserved.
#
# Licensed for non-commercial use with attribution only.
# Commercial use is strictly prohibited.

extends RefCounted

var tests_passed := 0
var tests_failed := 0


func run_all() -> void:
	tests_passed = 0
	tests_failed = 0

	print("\n=== IvyGenerator & IvySettings Tests ===")

	test_settings_properties()
	test_reject_incomplete_setup()
	test_generate_explicit_and_auto_starts()
	test_generate_determinism()
	test_failed_generate_clears_previous_results()
	test_preset_resource()
	test_target_points_steering_and_queries()
	test_target_points_guaranteed_reached()

	_print_test_summary()


func _make_uniform_image(value: float = 1.0) -> Image:
	var image := Image.create(4, 4, false, Image.FORMAT_L8)
	image.fill(Color(value, value, value))
	return image


func _run_test(test_name: String, result: bool) -> void:
	if result:
		tests_passed += 1
		print("PASS: ", test_name)
	else:
		tests_failed += 1
		print("FAIL: ", test_name)


func _print_test_summary() -> void:
	print("\n=== Test Summary ===")
	print("Passed: ", tests_passed)
	print("Failed: ", tests_failed)
	print("====================\n")


func test_settings_properties() -> void:
	var settings := IvySettings.new()

	_run_test("Default seed is 42", settings.seed == 42)
	_run_test("Default branch_angle_degrees is 30", is_equal_approx(settings.branch_angle_degrees, 30.0))
	_run_test("Default target_weight is 0", is_equal_approx(settings.target_weight, 0.0))
	_run_test("Default target_influence_radius is 0", is_equal_approx(settings.target_influence_radius, 0.0))
	_run_test("Default target_acceptance_radius is 0.2", is_equal_approx(settings.target_acceptance_radius, 0.2))

	settings.seed = 777
	settings.start_count = 4
	settings.max_steps = 50
	settings.branch_probability = 0.25
	settings.branch_angle_degrees = 45.0
	settings.thickness_growth_per_step = 0.01
	settings.wither_rate_per_step = 0.005
	settings.target_weight = 1.75
	settings.target_influence_radius = 2.5
	settings.target_acceptance_radius = 0.35

	_run_test("Settings seed round-trip", settings.seed == 777)
	_run_test("Settings start_count round-trip", settings.start_count == 4)
	_run_test("Settings max_steps round-trip", settings.max_steps == 50)
	_run_test("Settings branch_probability round-trip", is_equal_approx(settings.branch_probability, 0.25))
	_run_test("Settings branch_angle_degrees round-trip", is_equal_approx(settings.branch_angle_degrees, 45.0))
	_run_test("Settings thickness_growth_per_step round-trip", is_equal_approx(settings.thickness_growth_per_step, 0.01))
	_run_test("Settings wither_rate_per_step round-trip", is_equal_approx(settings.wither_rate_per_step, 0.005))
	_run_test("Settings target_weight round-trip", is_equal_approx(settings.target_weight, 1.75))
	_run_test("Settings target_influence_radius round-trip", is_equal_approx(settings.target_influence_radius, 2.5))
	_run_test("Settings target_acceptance_radius round-trip", is_equal_approx(settings.target_acceptance_radius, 0.35))


func test_reject_incomplete_setup() -> void:
	var generator := IvyGenerator.new()

	_run_test("Reject generate without setup", not generator.generate())

	var invalid_wall := generator.set_wall(Vector3.ZERO, Vector3.RIGHT, Vector3.UP, 0.0, 5.0)
	_run_test("Reject invalid zero-width wall", not invalid_wall)

	var valid_wall := generator.set_wall(Vector3.ZERO, Vector3.RIGHT, Vector3.UP, 10.0, 10.0)
	_run_test("Accept valid wall", valid_wall)

	var settings := IvySettings.new()
	settings.start_count = 1
	settings.max_steps = 10
	generator.settings = settings

	_run_test("Reject generate without attraction image", not generator.generate())
	_run_test("Accept valid attraction image", generator.set_attraction_image(_make_uniform_image()))

	# Too many explicit starts for start_count = 1.
	generator.set_start_points(PackedVector3Array([
		Vector3(1.0, 0.0, 0.0),
		Vector3(2.0, 0.0, 0.0),
	]))
	_run_test("Reject explicit starts exceeding start_count", not generator.generate())

	# Explicit start off wall plane.
	generator.set_start_points(PackedVector3Array([Vector3(1.0, 0.0, 5.0)]))
	_run_test("Reject off-wall explicit start point", not generator.generate())

	# Target point off wall plane.
	generator.clear_start_points()
	generator.set_target_points(PackedVector3Array([Vector3(1.0, 2.0, 5.0)]))
	_run_test("Reject off-wall target point", not generator.generate() and generator.get_target_count() == 0)


func test_generate_explicit_and_auto_starts() -> void:
	var generator := IvyGenerator.new()
	var settings := IvySettings.new()
	settings.seed = 42
	settings.start_count = 3
	settings.max_steps = 5
	settings.step_length = 0.5
	settings.initial_thickness = 0.1
	settings.thickness_growth_per_step = 0.01

	generator.settings = settings
	generator.set_wall(Vector3.ZERO, Vector3.RIGHT, Vector3.UP, 10.0, 10.0)
	generator.set_attraction_image(_make_uniform_image())
	generator.set_start_points(PackedVector3Array([Vector3(2.0, 0.0, 0.0)]))

	var ok := generator.generate()
	_run_test("Generate with explicit + auto starts succeeds", ok)
	if not ok:
		return

	var count := generator.get_point_count()
	var positions := generator.get_point_positions()
	var thicknesses := generator.get_point_thicknesses()
	var parents := generator.get_point_parent_indices()

	_run_test("Generated points exceed initial root count", count > 3)
	_run_test("Packed arrays match point count",
		positions.size() == count and thicknesses.size() == count and parents.size() == count)
	_run_test("First 3 points are roots",
		parents[0] == -1 and parents[1] == -1 and parents[2] == -1)
	_run_test("Explicit start preserved at index 0",
		positions[0].is_equal_approx(Vector3(2.0, 0.0, 0.0)))
	_run_test("Auto-generated starts lie on wall base",
		is_equal_approx(positions[1].y, 0.0) and is_equal_approx(positions[1].z, 0.0)
		and positions[1].x >= 0.0 and positions[1].x <= 10.0
		and is_equal_approx(positions[2].y, 0.0) and is_equal_approx(positions[2].z, 0.0)
		and positions[2].x >= 0.0 and positions[2].x <= 10.0)
	_run_test("Living roots thickened above initial_thickness",
		thicknesses[0] > settings.initial_thickness)


func test_generate_determinism() -> void:
	var image := _make_uniform_image()

	var make_run := func(run_seed: int) -> IvyGenerator:
		var gen := IvyGenerator.new()
		var cfg := IvySettings.new()
		cfg.seed = run_seed
		cfg.start_count = 2
		cfg.max_steps = 8
		cfg.step_length = 0.4
		cfg.random_weight = 0.4
		cfg.branch_probability = 0.3
		cfg.branch_angle_degrees = 35.0
		gen.settings = cfg
		gen.set_wall(Vector3.ZERO, Vector3.RIGHT, Vector3.UP, 10.0, 10.0)
		gen.set_attraction_image(image)
		gen.generate()
		return gen

	var run_a: IvyGenerator = make_run.call(1234)
	var run_b: IvyGenerator = make_run.call(1234)
	var run_c: IvyGenerator = make_run.call(5678)

	_run_test("Deterministic point count for identical seeds",
		run_a.get_point_count() > 2 and run_a.get_point_count() == run_b.get_point_count())
	_run_test("Deterministic positions for identical seeds",
		run_a.get_point_positions() == run_b.get_point_positions())
	_run_test("Deterministic thicknesses for identical seeds",
		run_a.get_point_thicknesses() == run_b.get_point_thicknesses())
	_run_test("Deterministic parent indices for identical seeds",
		run_a.get_point_parent_indices() == run_b.get_point_parent_indices())
	_run_test("Different seed produces different positions",
		run_a.get_point_positions() != run_c.get_point_positions())


func test_failed_generate_clears_previous_results() -> void:
	var generator := IvyGenerator.new()
	var settings := IvySettings.new()
	settings.max_steps = 5

	generator.settings = settings
	generator.set_wall(Vector3.ZERO, Vector3.RIGHT, Vector3.UP, 10.0, 10.0)
	generator.set_attraction_image(_make_uniform_image())

	_run_test("Initial valid generate succeeds", generator.generate() and generator.get_point_count() > 0)

	generator.settings = null
	_run_test("Subsequent invalid generate fails", not generator.generate())
	_run_test("Failed generate clears previous results",
		generator.get_point_count() == 0 and generator.get_point_positions().is_empty())


func test_preset_resource() -> void:
	var preset := ResourceLoader.load("res://presets/ivy_preset.tres", "IvySettings")
	_run_test("Preset resource loads as IvySettings", preset is IvySettings)
	if preset is IvySettings:
		_run_test("Preset seed matches expected value", preset.seed == 138)
		var generator := IvyGenerator.new()
		generator.settings = preset
		generator.set_wall(Vector3.ZERO, Vector3.RIGHT, Vector3.UP, 6.0, 4.0)
		generator.set_attraction_image(_make_uniform_image())
		_run_test("Generate with preset resource succeeds", generator.generate() and generator.get_point_count() > 0)


func test_target_points_steering_and_queries() -> void:
	var generator := IvyGenerator.new()
	var settings := IvySettings.new()
	settings.seed = 42
	settings.start_count = 1
	settings.max_steps = 8
	settings.step_length = 0.5
	settings.momentum_weight = 0.5
	settings.upward_weight = 0.5
	settings.attraction_weight = 0.0
	settings.contour_weight = 0.0
	settings.target_weight = 2.0
	settings.target_influence_radius = 0.0
	settings.target_acceptance_radius = 0.35

	generator.settings = settings
	generator.set_wall(Vector3.ZERO, Vector3.RIGHT, Vector3.UP, 10.0, 10.0)
	generator.set_attraction_image(_make_uniform_image())
	generator.set_start_points(PackedVector3Array([Vector3(5.0, 0.0, 0.0)]))

	var target := Vector3(6.0, 1.5, 0.0)
	generator.set_target_points(PackedVector3Array([target]))
	var stored_targets := generator.get_target_points()
	_run_test("Configured target points round-trip",
		stored_targets.size() == 1 and stored_targets[0].is_equal_approx(target))

	var ok := generator.generate()
	_run_test("Generate with target waypoint succeeds", ok)
	if not ok:
		return

	_run_test("Target count matches configured targets", generator.get_target_count() == 1)
	_run_test("Target waypoint marked reached", generator.is_target_reached(0))
	_run_test("Reached target count is 1", generator.get_reached_target_count() == 1)
	_run_test("Out-of-range target index returns false",
		not generator.is_target_reached(-1) and not generator.is_target_reached(1))

	var positions := generator.get_point_positions()
	_run_test("Soft waypoint lets vine keep growing past target",
		positions.size() == 9 and positions[positions.size() - 1].y > 1.8)

	generator.clear_target_points()
	_run_test("Clear target points empties configured list", generator.get_target_points().is_empty())

	generator.settings = null
	_run_test("Failed generate resets reached target state",
		not generator.generate() and generator.get_target_count() == 0 and generator.get_reached_target_count() == 0)


func test_target_points_guaranteed_reached() -> void:
	var generator := IvyGenerator.new()
	var settings := IvySettings.new()
	settings.seed = 42
	settings.start_count = 1
	settings.max_steps = 6
	settings.step_length = 0.4
	settings.target_weight = 0.0
	settings.target_acceptance_radius = 0.25

	generator.settings = settings
	generator.set_wall(Vector3.ZERO, Vector3.RIGHT, Vector3.UP, 10.0, 10.0)
	generator.set_attraction_image(_make_uniform_image())
	generator.set_start_points(PackedVector3Array([Vector3(1.0, 0.0, 0.0)]))
	generator.set_target_points(PackedVector3Array([
		Vector3(8.5, 2.0, 0.0),
		Vector3(8.5, 7.5, 0.0),
	]))

	var ok := generator.generate()
	_run_test("Generate guarantees reaching distant unguided targets", ok)
	if not ok:
		return

	_run_test("Guaranteed reach target count is 2", generator.get_target_count() == 2)
	_run_test("Guaranteed reach reports both targets reached", generator.get_reached_target_count() == 2)
	_run_test("First distant target reached", generator.is_target_reached(0))
	_run_test("Second distant target reached", generator.is_target_reached(1))
