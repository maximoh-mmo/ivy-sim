# Copyright (c) 2026 Max Heinze. All rights reserved.
#
# Licensed for non-commercial use with attribution only.
# Commercial use is strictly prohibited.

extends Node3D

@export var settings: IvySettings
@export var mesh_settings: IvyMeshSettings
@export var wall_width: float = 6.0
@export var wall_height: float = 4.0
@export var uv_scale: Vector2 = Vector2(2.0, 2.0)

var _generator := IvyGenerator.new()
var _mesh_builder := IvyMeshBuilder.new()
var _attraction_image: Image

var _stem_mesh_instance: MeshInstance3D
var _leaf_multimesh_instance: MultiMeshInstance3D
var _target_mesh_instance: MeshInstance3D
var _settings_panel: IvySettingsPanel
var _camera: Camera3D
var _target_points := PackedVector3Array()

var _orbit_target := Vector3.ZERO
var _orbit_yaw := 0.0
var _orbit_pitch := 0.05
var _orbit_distance := 7.5
var _is_orbiting := false
var _show_leaves := true


func _ready() -> void:
	if settings == null:
		settings = _create_default_settings()
	if mesh_settings == null:
		mesh_settings = _create_default_mesh_settings()

	_attraction_image = _create_brick_attraction_image(128, 128)
	_setup_scene()
	_setup_ui()
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
	s.target_weight = 0.6
	s.target_acceptance_radius = 0.2
	s.max_top_edge_steps = 15
	return s


func _create_default_mesh_settings() -> IvyMeshSettings:
	var m := IvyMeshSettings.new()
	m.radial_segments = 6
	m.radius_scale = 0.5
	m.wall_offset_ratio = 0.85
	m.uv_v_scale = 1.0
	m.cap_tips = true
	m.leaf_seed = 42
	m.leaf_density = 5.0
	m.leaf_min_thickness = 0.025
	m.leaf_max_thickness = 0.09
	m.leaf_size_min = 0.09
	m.leaf_size_max = 0.17
	m.leaf_tilt_degrees = 30.0
	m.leaf_yaw_variation_degrees = 30.0
	m.leaf_roll_variation_degrees = 20.0
	return m


func _create_brick_attraction_image(width: int, height: int) -> Image:
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

			var dist_mortar := minf(dist_h, dist_v * (rows / cols))
			var attraction := clampf(1.0 - dist_mortar * 3.5, 0.05, 1.0)
			img.set_pixel(x, y, Color(attraction, attraction, attraction))

	return img


func _create_stem_3d_material() -> ShaderMaterial:
	var shader := Shader.new()
	shader.code = """
shader_type spatial;
void fragment() {
	vec3 young_shoot = vec3(0.24, 0.50, 0.16);
	vec3 woody_bark = vec3(0.29, 0.21, 0.13);
	float grain = 0.92 + 0.08 * sin(UV.x * 37.699 + UV.y * 6.0);
	ALBEDO = mix(young_shoot, woody_bark, smoothstep(0.15, 0.75, COLOR.r)) * grain;
	ROUGHNESS = mix(0.65, 0.90, COLOR.r);
}
"""
	var mat := ShaderMaterial.new()
	mat.shader = shader
	return mat


func _create_placeholder_leaf_mesh() -> ArrayMesh:
	# Local convention: origin = petiole, -Z = blade tip, +Y = upper leaf face normal.
	var st := SurfaceTool.new()
	st.begin(Mesh.PRIMITIVE_TRIANGLES)

	var p_base := Vector3(0.0, 0.0, 0.0)
	var p_mid := Vector3(0.0, 0.03, -0.45)
	var p_tip := Vector3(0.0, -0.02, -1.0)
	var p_l_back := Vector3(-0.32, 0.06, -0.18)
	var p_l_side := Vector3(-0.46, 0.08, -0.52)
	var p_r_back := Vector3(0.32, 0.06, -0.18)
	var p_r_side := Vector3(0.46, 0.08, -0.52)

	var tris := [
		[p_base, p_l_back, p_mid],
		[p_l_back, p_l_side, p_mid],
		[p_l_side, p_tip, p_mid],
		[p_base, p_mid, p_r_back],
		[p_mid, p_r_side, p_r_back],
		[p_mid, p_tip, p_r_side],
	]
	for tri in tris:
		for v: Vector3 in tri:
			st.add_vertex(v)

	st.generate_normals()
	return st.commit()


func _setup_scene() -> void:
	var env := Environment.new()
	env.ambient_light_source = Environment.AMBIENT_SOURCE_COLOR
	env.ambient_light_color = Color(0.55, 0.58, 0.62)
	env.ambient_light_energy = 0.6
	var world_env := WorldEnvironment.new()
	world_env.environment = env
	add_child(world_env)

	var light := DirectionalLight3D.new()
	light.rotation_degrees = Vector3(-35.0, 25.0, 0.0)
	add_child(light)

	_camera = Camera3D.new()
	add_child(_camera)
	_reset_camera_orbit()

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

	# 3D stem cylinder mesh (back-face culling enabled to verify CW winding)
	_stem_mesh_instance = MeshInstance3D.new()
	_stem_mesh_instance.material_override = _create_stem_3d_material()
	add_child(_stem_mesh_instance)

	# Instanced 3D leaves via MultiMeshInstance3D
	var leaf_mat := StandardMaterial3D.new()
	leaf_mat.albedo_color = Color(0.17, 0.42, 0.14)
	leaf_mat.roughness = 0.65
	leaf_mat.cull_mode = BaseMaterial3D.CULL_DISABLED

	var leaf_mm := MultiMesh.new()
	leaf_mm.transform_format = MultiMesh.TRANSFORM_3D
	leaf_mm.mesh = _create_placeholder_leaf_mesh()

	_leaf_multimesh_instance = MultiMeshInstance3D.new()
	_leaf_multimesh_instance.multimesh = leaf_mm
	_leaf_multimesh_instance.material_override = leaf_mat
	add_child(_leaf_multimesh_instance)

	# Target waypoint crosshairs & acceptance rings
	var marker_mat := StandardMaterial3D.new()
	marker_mat.vertex_color_use_as_albedo = true
	marker_mat.shading_mode = BaseMaterial3D.SHADING_MODE_UNSHADED
	_target_mesh_instance = MeshInstance3D.new()
	_target_mesh_instance.material_override = marker_mat
	add_child(_target_mesh_instance)


func _reset_camera_orbit() -> void:
	_orbit_target = Vector3(wall_width * 0.35, wall_height * 0.5, 0.0)
	_orbit_yaw = 0.0
	_orbit_pitch = 0.05
	_orbit_distance = maxf(wall_width, wall_height) * 1.25
	_update_camera_transform()


func _update_camera_transform() -> void:
	if _camera == null:
		return
	var cos_pitch := cos(_orbit_pitch)
	var offset := Vector3(
		sin(_orbit_yaw) * cos_pitch,
		sin(_orbit_pitch),
		cos(_orbit_yaw) * cos_pitch
	) * _orbit_distance
	_camera.position = _orbit_target + offset
	_camera.look_at(_orbit_target, Vector3.UP)


func _setup_ui() -> void:
	_settings_panel = IvySettingsPanel.new()
	add_child(_settings_panel)
	_settings_panel.setup(settings)
	_settings_panel.settings_changed.connect(_regenerate)
	_settings_panel.reset_requested.connect(func() -> void:
		settings = _create_default_settings()
		_settings_panel.set_settings(settings)
		_regenerate()
	)
	_settings_panel.preset_loaded.connect(func(loaded: IvySettings) -> void:
		settings = loaded
		_regenerate()
	)


func _regenerate() -> void:
	_generator.settings = settings
	_generator.set_wall(Vector3.ZERO, Vector3.RIGHT, Vector3.UP, wall_width, wall_height, uv_scale, Vector2.ZERO)
	_generator.set_attraction_image(_attraction_image)
	_generator.set_target_points(_target_points)

	var sim_start_us := Time.get_ticks_usec()
	if not _generator.generate():
		_stem_mesh_instance.mesh = null
		_leaf_multimesh_instance.multimesh.instance_count = 0
		if _settings_panel != null:
			_settings_panel.set_status("Invalid settings combination (check thickness/weight bounds)", true)
		return
	var sim_ms := float(Time.get_ticks_usec() - sim_start_us) / 1000.0

	var positions := _generator.get_point_positions()
	var thicknesses := _generator.get_point_thicknesses()
	var parents := _generator.get_point_parent_indices()

	var mesh_start_us := Time.get_ticks_usec()
	mesh_settings.leaf_seed = settings.seed
	_mesh_builder.settings = mesh_settings
	_stem_mesh_instance.mesh = _mesh_builder.build_stem_mesh(
		positions, thicknesses, parents, Vector3.BACK)

	var leaf_transforms := _mesh_builder.build_leaf_transforms(
		positions, thicknesses, parents, Vector3.BACK)
	var leaf_count := leaf_transforms.size()
	var mm := _leaf_multimesh_instance.multimesh
	mm.instance_count = leaf_count
	for i in range(leaf_count):
		mm.set_instance_transform(i, leaf_transforms[i])
	_leaf_multimesh_instance.visible = _show_leaves
	var mesh_ms := float(Time.get_ticks_usec() - mesh_start_us) / 1000.0

	var reached_flags: Array[bool] = []
	for i in range(_target_points.size()):
		reached_flags.append(_generator.is_target_reached(i))

	_target_mesh_instance.mesh = IvyDebugRenderer.build_target_markers_mesh(
		_target_points,
		reached_flags,
		settings.target_acceptance_radius,
		Vector3.BACK
	)

	var status := "[3D] Seed: %d | Pts: %d | Leaves: %d | Targets: %d/%d | Sim: %.2f ms | Mesh: %.2f ms" % [
		settings.seed,
		_generator.get_point_count(),
		leaf_count,
		_generator.get_reached_target_count(),
		_generator.get_target_count(),
		sim_ms,
		mesh_ms
	]
	if _settings_panel != null:
		_settings_panel.set_status(status)


func _unhandled_input(event: InputEvent) -> void:
	if event is InputEventKey and event.pressed and not event.echo:
		if event.keycode == KEY_SPACE:
			settings.seed += 1
			if _settings_panel != null:
				_settings_panel.sync_from_settings()
			_regenerate()
		elif event.keycode == KEY_C and not _target_points.is_empty():
			_target_points.clear()
			_regenerate()
		elif event.keycode == KEY_L:
			_show_leaves = not _show_leaves
			_leaf_multimesh_instance.visible = _show_leaves
		elif event.keycode == KEY_R:
			_reset_camera_orbit()
	elif event is InputEventMouseMotion and _is_orbiting:
		_orbit_yaw -= event.relative.x * 0.008
		_orbit_pitch = clampf(_orbit_pitch + event.relative.y * 0.008, -1.35, 1.35)
		_update_camera_transform()
	elif event is InputEventMouseButton:
		var orbit_btn: bool = event.button_index == MOUSE_BUTTON_MIDDLE \
			or (event.button_index == MOUSE_BUTTON_LEFT and event.alt_pressed)
		if orbit_btn:
			if event.pressed and event.position.x >= 330.0:
				_is_orbiting = true
			elif not event.pressed:
				_is_orbiting = false
			return

		if event.pressed and event.position.x >= 330.0:
			if event.button_index == MOUSE_BUTTON_WHEEL_UP:
				_orbit_distance = maxf(_orbit_distance * 0.9, 1.2)
				_update_camera_transform()
				return
			elif event.button_index == MOUSE_BUTTON_WHEEL_DOWN:
				_orbit_distance = minf(_orbit_distance * 1.1, 20.0)
				_update_camera_transform()
				return

		if event.pressed and _camera != null and event.position.x >= 330.0:
			var hit: Variant = _raycast_wall(event.position)
			if hit == null:
				return
			var hit_pos: Vector3 = hit
			if event.button_index == MOUSE_BUTTON_LEFT:
				_target_points.append(hit_pos)
				_regenerate()
			elif event.button_index == MOUSE_BUTTON_RIGHT:
				_remove_nearest_target(hit_pos)
				_regenerate()


func _raycast_wall(screen_pos: Vector2) -> Variant:
	var ray_origin := _camera.project_ray_origin(screen_pos)
	var ray_dir := _camera.project_ray_normal(screen_pos)
	var wall_plane := Plane(Vector3.BACK, 0.0)
	var hit: Variant = wall_plane.intersects_ray(ray_origin, ray_dir)
	if hit == null:
		return null
	var hit_pos: Vector3 = hit
	if hit_pos.x < 0.0 or hit_pos.x > wall_width or hit_pos.y < 0.0 or hit_pos.y > wall_height:
		return null
	return Vector3(hit_pos.x, hit_pos.y, 0.0)


func _remove_nearest_target(point: Vector3) -> void:
	var best_idx := -1
	var max_dist := maxf(settings.target_acceptance_radius, 0.35)
	var best_dist_sq := max_dist * max_dist
	for i in range(_target_points.size()):
		var d_sq := _target_points[i].distance_squared_to(point)
		if d_sq <= best_dist_sq:
			best_dist_sq = d_sq
			best_idx = i
	if best_idx >= 0:
		_target_points.remove_at(best_idx)
