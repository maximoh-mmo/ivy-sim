# Copyright (c) 2026 Max Heinze. All rights reserved.
#
# Licensed for non-commercial use with attribution only.
# Commercial use is strictly prohibited.

extends Node

@export var settings: IvySettings
@export var wall_width: float = 6.0
@export var wall_height: float = 4.0
@export var uv_scale: Vector2 = Vector2(2.0, 2.0)

var _generator := IvyGenerator.new()
var _attraction_image: Image
var _ivy_mesh_instance: MeshInstance3D


func _ready() -> void:
	if settings == null:
		settings = _create_default_settings()

	_attraction_image = _create_brick_attraction_image(128, 128)
	_setup_scene()
	_regenerate()


func _unhandled_input(event: InputEvent) -> void:
	if event is InputEventKey and event.pressed and not event.echo and event.keycode == KEY_SPACE:
		settings.seed += 1
		_regenerate()


func _create_default_settings() -> IvySettings:
	var s := IvySettings.new()
	s.seed = 42
	s.start_count = 3
	s.max_steps = 80
	s.step_length = 0.08
	s.initial_thickness = 0.03
	s.max_thickness = 0.14
	s.thickness_growth_per_step = 0.0015
	s.wither_rate_per_step = 0.001
	s.min_withered_thickness = 0.01
	s.momentum_weight = 1.0
	s.upward_weight = 0.55
	s.random_weight = 0.35
	s.attraction_weight = 0.8
	s.contour_weight = 0.6
	s.gradient_epsilon = 0.015
	s.branch_probability = 0.08
	s.branch_angle_degrees = 32.0
	s.max_points = 4000
	s.min_branch_interval = 6
	s.crowding_radius = 0.25
	s.avoidance_weight = 0.8
	s.max_crowding_neighbors = 5
	return s


func _create_brick_attraction_image(width: int, height: int) -> Image:
	# Grayscale brick-and-mortar field with smooth gradient falloff into mortar joints.
	var img := Image.create(width, height, false, Image.FORMAT_L8)
	var rows := 4.0
	var cols := 2.0

	for y in range(height):
		var v := 1.0 - (float(y) + 0.5) / float(height)
		var row_f := v * rows
		var row_idx := int(floor(row_f))
		var row_local := row_f - float(row_idx)
		var dist_h := minf(row_local, 1.0 - row_local)

		for x in range(width):
			var u := (float(x) + 0.5) / float(width)
			var stagger := 0.5 if (row_idx % 2 == 1) else 0.0
			var col_f := fposmod(u * cols + stagger, cols)
			var col_local := col_f - floorf(col_f)
			var dist_v := minf(col_local, 1.0 - col_local)

			# Smoothly fall off from 1.0 in mortar grooves toward 0.05 at brick centers.
			var dist_mortar := minf(dist_h, dist_v * (rows / cols))
			var attraction := clampf(1.0 - dist_mortar * 3.5, 0.05, 1.0)
			img.set_pixel(x, y, Color(attraction, attraction, attraction))

	return img


func _setup_scene() -> void:
	# Lighting
	var light := DirectionalLight3D.new()
	light.rotation_degrees = Vector3(-35.0, 25.0, 0.0)
	add_child(light)

	# Camera centered on the wall
	var camera := Camera3D.new()
	camera.position = Vector3(wall_width * 0.5, wall_height * 0.5, maxf(wall_width, wall_height) * 1.15)
	add_child(camera)
	camera.look_at(Vector3(wall_width * 0.5, wall_height * 0.5, 0.0), Vector3.UP)

	# Planar wall mesh aligned with IvyWall (bottom-left at origin, facing +Z)
	var wall_mesh := QuadMesh.new()
	wall_mesh.size = Vector2(wall_width, wall_height)

	var wall_mat := StandardMaterial3D.new()
	wall_mat.albedo_texture = ImageTexture.create_from_image(_attraction_image)
	wall_mat.uv1_scale = Vector3(uv_scale.x, uv_scale.y, 1.0)
	wall_mat.texture_filter = BaseMaterial3D.TEXTURE_FILTER_LINEAR

	var wall_instance := MeshInstance3D.new()
	wall_instance.mesh = wall_mesh
	wall_instance.material_override = wall_mat
	wall_instance.position = Vector3(wall_width * 0.5, wall_height * 0.5, 0.0)
	add_child(wall_instance)

	# Ivy debug ribbon mesh
	var ivy_mat := StandardMaterial3D.new()
	ivy_mat.vertex_color_use_as_albedo = true
	ivy_mat.shading_mode = BaseMaterial3D.SHADING_MODE_UNSHADED
	ivy_mat.cull_mode = BaseMaterial3D.CULL_DISABLED

	_ivy_mesh_instance = MeshInstance3D.new()
	_ivy_mesh_instance.material_override = ivy_mat
	add_child(_ivy_mesh_instance)


func _regenerate() -> void:
	_generator.settings = settings
	_generator.set_wall(Vector3.ZERO, Vector3.RIGHT, Vector3.UP, wall_width, wall_height, uv_scale, Vector2.ZERO)
	_generator.set_attraction_image(_attraction_image)

	if not _generator.generate():
		push_error("IvyGenerator.generate() failed.")
		return

	_ivy_mesh_instance.mesh = IvyDebugRenderer.build_ribbon_mesh(
		_generator.get_point_positions(),
		_generator.get_point_thicknesses(),
		_generator.get_point_parent_indices(),
		Vector3.BACK,
		settings.initial_thickness
	)

	print("Generated ivy | seed=%d | points=%d" % [settings.seed, _generator.get_point_count()])
