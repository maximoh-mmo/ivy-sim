# Copyright (c) 2026 Max Heinze. All rights reserved.
#
# Licensed for non-commercial use with attribution only.
# Commercial use is strictly prohibited.

extends Node2D

@export var settings: IvySettings
@export var wall_width: float = 6.0
@export var wall_height: float = 4.0
@export var uv_scale: Vector2 = Vector2(2.0, 2.0)

var _generator := IvyGenerator.new()
var _attraction_image: Image
var _attraction_texture: ImageTexture
var _settings_panel: IvySettingsPanel
var _target_points := PackedVector3Array()

var _positions := PackedVector3Array()
var _thicknesses := PackedFloat32Array()
var _parent_indices := PackedInt32Array()
var _reached_flags: Array[bool] = []
var _board_rect := Rect2()


func _ready() -> void:
	if settings == null:
		settings = _create_default_settings()

	texture_repeat = CanvasItem.TEXTURE_REPEAT_ENABLED
	texture_filter = CanvasItem.TEXTURE_FILTER_LINEAR

	_attraction_image = _create_brick_attraction_image(128, 128)
	_attraction_texture = ImageTexture.create_from_image(_attraction_image)

	_setup_ui()
	_update_board_rect()
	get_viewport().size_changed.connect(_on_viewport_resized)
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


func _on_viewport_resized() -> void:
	_update_board_rect()
	queue_redraw()


func _update_board_rect() -> void:
	var vp_size := get_viewport_rect().size
	var left_panel := 346.0
	var pad := 24.0
	var avail_w := maxf(vp_size.x - left_panel - pad, 100.0)
	var avail_h := maxf(vp_size.y - pad * 2.0, 100.0)

	var aspect := wall_width / maxf(wall_height, 0.001)
	var rect_w := avail_w
	var rect_h := rect_w / aspect
	if rect_h > avail_h:
		rect_h = avail_h
		rect_w = rect_h * aspect

	var pos_x := left_panel + (avail_w - rect_w) * 0.5
	var pos_y := pad + (avail_h - rect_h) * 0.5
	_board_rect = Rect2(pos_x, pos_y, rect_w, rect_h)


func _wall_to_canvas(wall_pos: Vector3) -> Vector2:
	return Vector2(
		_board_rect.position.x + (wall_pos.x / wall_width) * _board_rect.size.x,
		_board_rect.position.y + (1.0 - wall_pos.y / wall_height) * _board_rect.size.y
	)


func _canvas_to_wall(canvas_pos: Vector2) -> Variant:
	if not _board_rect.has_point(canvas_pos):
		return null
	var u := (canvas_pos.x - _board_rect.position.x) / _board_rect.size.x
	var v := 1.0 - (canvas_pos.y - _board_rect.position.y) / _board_rect.size.y
	return Vector3(u * wall_width, v * wall_height, 0.0)


func _regenerate() -> void:
	_generator.settings = settings
	_generator.set_wall(Vector3.ZERO, Vector3.RIGHT, Vector3.UP, wall_width, wall_height, uv_scale, Vector2.ZERO)
	_generator.set_attraction_image(_attraction_image)
	_generator.set_target_points(_target_points)

	var start_us := Time.get_ticks_usec()
	if not _generator.generate():
		_positions = PackedVector3Array()
		_thicknesses = PackedFloat32Array()
		_parent_indices = PackedInt32Array()
		_reached_flags.clear()
		if _settings_panel != null:
			_settings_panel.set_status("Invalid settings combination (check thickness/weight bounds)", true)
		queue_redraw()
		return
	var elapsed_ms := float(Time.get_ticks_usec() - start_us) / 1000.0

	_positions = _generator.get_point_positions()
	_thicknesses = _generator.get_point_thicknesses()
	_parent_indices = _generator.get_point_parent_indices()

	_reached_flags.clear()
	for i in range(_target_points.size()):
		_reached_flags.append(_generator.is_target_reached(i))

	var status := "[2D] Seed: %d | Points: %d | Targets: %d/%d | %.2f ms" % [
		settings.seed,
		_generator.get_point_count(),
		_generator.get_reached_target_count(),
		_generator.get_target_count(),
		elapsed_ms
	]
	if _settings_panel != null:
		_settings_panel.set_status(status)
	queue_redraw()


func _draw() -> void:
	if _board_rect.size.x <= 0.0 or _board_rect.size.y <= 0.0:
		return

	if _attraction_texture != null:
		var tex_size := Vector2(_attraction_texture.get_size())
		var src_rect := Rect2(Vector2.ZERO, Vector2(tex_size.x * uv_scale.x, tex_size.y * uv_scale.y))
		draw_texture_rect_region(_attraction_texture, _board_rect, src_rect, Color(0.85, 0.85, 0.85))
	draw_rect(_board_rect, Color(0.5, 0.55, 0.5), false, 1.5)

	var px_per_unit := _board_rect.size.x / maxf(wall_width, 0.001)
	var count := _positions.size()

	for i in range(count):
		var parent_idx := _parent_indices[i]
		if parent_idx < 0 or parent_idx >= count:
			continue

		var p0 := _wall_to_canvas(_positions[parent_idx])
		var p1 := _wall_to_canvas(_positions[i])
		var seg := p1 - p0
		if seg.length_squared() <= 0.001:
			continue

		var side := Vector2(-seg.y, seg.x).normalized()
		var r0 := _thicknesses[parent_idx] * 0.5 * px_per_unit
		var r1 := _thicknesses[i] * 0.5 * px_per_unit
		var c0 := _thickness_color(_thicknesses[parent_idx], settings.initial_thickness)
		var c1 := _thickness_color(_thicknesses[i], settings.initial_thickness)

		var quad := PackedVector2Array([
			p0 - side * r0,
			p0 + side * r0,
			p1 + side * r1,
			p1 - side * r1,
		])
		var colors := PackedColorArray([c0, c0, c1, c1])
		draw_polygon(quad, colors)

	# Draw target crosshairs and acceptance radii
	var accept_px := settings.target_acceptance_radius * px_per_unit
	var cross_px := minf(accept_px * 0.35, 10.0)
	for i in range(_target_points.size()):
		var center := _wall_to_canvas(_target_points[i])
		var reached := i < _reached_flags.size() and _reached_flags[i]
		var color := Color(0.2, 1.0, 0.85) if reached else Color(1.0, 0.4, 0.2)
		draw_line(center - Vector2(cross_px, 0.0), center + Vector2(cross_px, 0.0), color, 1.5)
		draw_line(center - Vector2(0.0, cross_px), center + Vector2(0.0, cross_px), color, 1.5)
		draw_arc(center, accept_px, 0.0, TAU, 32, color, 1.5)


func _thickness_color(thickness: float, initial_thickness: float) -> Color:
	if thickness < initial_thickness - 0.0005:
		return Color(0.55, 0.42, 0.25)
	var maturity := clampf((thickness - initial_thickness) / maxf(initial_thickness * 2.0, 0.001), 0.0, 1.0)
	return Color(0.25, 0.80, 0.25).lerp(Color(0.28, 0.36, 0.16), maturity)


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
	elif event is InputEventMouseButton and event.pressed:
		var hit: Variant = _canvas_to_wall(event.position)
		if hit == null:
			return
		var hit_pos: Vector3 = hit
		if event.button_index == MOUSE_BUTTON_LEFT:
			_target_points.append(hit_pos)
			_regenerate()
		elif event.button_index == MOUSE_BUTTON_RIGHT:
			_remove_nearest_target(hit_pos)
			_regenerate()


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
