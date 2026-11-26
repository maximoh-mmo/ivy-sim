# Copyright (c) 2026 Max Heinze. All rights reserved.
#
# Licensed for non-commercial use with attribution only.
# Commercial use is strictly prohibited.

class_name IvySettingsPanel
extends CanvasLayer

signal settings_changed
signal mesh_settings_changed
signal reset_requested
signal preset_loaded(new_settings: IvySettings)

var _settings: IvySettings
var _mesh_settings: IvyMeshSettings
var _growth_controls: Dictionary = {} # property_name -> Array[Control]
var _mesh_controls: Dictionary = {} # property_name -> Array[Control]
var _updating_ui := false

var _status_label: Label
var _save_dialog: FileDialog
var _load_dialog: FileDialog


func setup(settings: IvySettings, mesh_settings: IvyMeshSettings = null) -> void:
	_settings = settings
	_mesh_settings = mesh_settings
	_build_ui()
	sync_from_settings()


func set_settings(settings: IvySettings) -> void:
	_settings = settings
	sync_from_settings()


func set_mesh_settings(mesh_settings: IvyMeshSettings) -> void:
	_mesh_settings = mesh_settings
	sync_from_settings()


func set_status(text: String, is_error: bool = false) -> void:
	if _status_label == null:
		return
	_status_label.text = text
	_status_label.modulate = Color(1.0, 0.45, 0.45) if is_error else Color(0.85, 0.95, 0.85)


func sync_from_settings() -> void:
	_updating_ui = true
	_sync_control_map(_settings, _growth_controls)
	_sync_control_map(_mesh_settings, _mesh_controls)
	_updating_ui = false


func _sync_control_map(target: Object, controls_map: Dictionary) -> void:
	if target == null:
		return
	for prop_name: String in controls_map:
		var val: Variant = target.get(prop_name)
		for ctrl: Control in controls_map[prop_name]:
			if ctrl is Range:
				(ctrl as Range).value = float(val)
			elif ctrl is CheckBox:
				(ctrl as CheckBox).button_pressed = bool(val)


func _build_ui() -> void:
	var panel := PanelContainer.new()
	panel.set_anchors_preset(Control.PRESET_LEFT_WIDE)
	panel.custom_minimum_size = Vector2(330.0, 0.0)
	panel.offset_right = 330.0
	add_child(panel)

	var margin := MarginContainer.new()
	margin.add_theme_constant_override("margin_left", 10)
	margin.add_theme_constant_override("margin_right", 10)
	margin.add_theme_constant_override("margin_top", 10)
	margin.add_theme_constant_override("margin_bottom", 10)
	panel.add_child(margin)

	var root_vbox := VBoxContainer.new()
	root_vbox.add_theme_constant_override("separation", 8)
	margin.add_child(root_vbox)

	# Title & status
	var title := Label.new()
	title.text = "IvySettings Editor"
	title.add_theme_font_size_override("font_size", 18)
	root_vbox.add_child(title)

	_status_label = Label.new()
	_status_label.text = "Ready"
	_status_label.autowrap_mode = TextServer.AUTOWRAP_WORD_SMART
	root_vbox.add_child(_status_label)

	# Action buttons
	var btn_grid := GridContainer.new()
	btn_grid.columns = 2
	btn_grid.add_theme_constant_override("h_separation", 6)
	btn_grid.add_theme_constant_override("v_separation", 6)
	root_vbox.add_child(btn_grid)

	var next_seed_btn := Button.new()
	next_seed_btn.text = "Next Seed (Space)"
	next_seed_btn.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	next_seed_btn.pressed.connect(func() -> void:
		if _settings == null:
			return
		_settings.seed += 1
		sync_from_settings()
		settings_changed.emit()
	)
	btn_grid.add_child(next_seed_btn)

	var reset_btn := Button.new()
	reset_btn.text = "Reset Defaults"
	reset_btn.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	reset_btn.pressed.connect(func() -> void:
		reset_requested.emit()
	)
	btn_grid.add_child(reset_btn)

	var save_btn := Button.new()
	save_btn.text = "Save Preset (.tres)"
	save_btn.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	save_btn.pressed.connect(func() -> void:
		_ensure_presets_dir()
		_save_dialog.popup_centered_ratio(0.55)
	)
	btn_grid.add_child(save_btn)

	var load_btn := Button.new()
	load_btn.text = "Load Preset (.tres)"
	load_btn.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	load_btn.pressed.connect(func() -> void:
		_ensure_presets_dir()
		_load_dialog.popup_centered_ratio(0.55)
	)
	btn_grid.add_child(load_btn)

	root_vbox.add_child(HSeparator.new())

	if _mesh_settings == null:
		var scroll := _create_scroll_container()
		root_vbox.add_child(scroll)
		var props_vbox := _create_props_vbox(scroll)
		_populate_property_list(props_vbox, &"IvySettings", _growth_controls, false)
	else:
		var tabs := TabContainer.new()
		tabs.size_flags_vertical = Control.SIZE_EXPAND_FILL
		root_vbox.add_child(tabs)

		var growth_scroll := _create_scroll_container()
		growth_scroll.name = "Growth"
		tabs.add_child(growth_scroll)
		var growth_vbox := _create_props_vbox(growth_scroll)
		_populate_property_list(growth_vbox, &"IvySettings", _growth_controls, false)

		var mesh_scroll := _create_scroll_container()
		mesh_scroll.name = "Mesh & Leaves"
		tabs.add_child(mesh_scroll)
		var mesh_vbox := _create_props_vbox(mesh_scroll)
		_populate_property_list(mesh_vbox, &"IvyMeshSettings", _mesh_controls, true)

	# Save / Load file dialogs (saving under res://presets/ makes .tres files usable in the Editor)
	_save_dialog = FileDialog.new()
	_save_dialog.file_mode = FileDialog.FILE_MODE_SAVE_FILE
	_save_dialog.access = FileDialog.ACCESS_RESOURCES
	_save_dialog.current_dir = "res://presets"
	_save_dialog.current_file = "ivy_preset.tres"
	_save_dialog.filters = PackedStringArray(["*.tres ; IvySettings Resource", "*.res ; Binary Resource"])
	_save_dialog.file_selected.connect(_on_save_preset)
	add_child(_save_dialog)

	_load_dialog = FileDialog.new()
	_load_dialog.file_mode = FileDialog.FILE_MODE_OPEN_FILE
	_load_dialog.access = FileDialog.ACCESS_RESOURCES
	_load_dialog.current_dir = "res://presets"
	_load_dialog.filters = PackedStringArray(["*.tres ; IvySettings Resource", "*.res ; Binary Resource"])
	_load_dialog.file_selected.connect(_on_load_preset)
	add_child(_load_dialog)


func _create_scroll_container() -> ScrollContainer:
	var scroll := ScrollContainer.new()
	scroll.size_flags_vertical = Control.SIZE_EXPAND_FILL
	scroll.horizontal_scroll_mode = ScrollContainer.SCROLL_MODE_DISABLED
	return scroll


func _create_props_vbox(scroll: ScrollContainer) -> VBoxContainer:
	var props_vbox := VBoxContainer.new()
	props_vbox.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	props_vbox.add_theme_constant_override("separation", 6)
	scroll.add_child(props_vbox)
	return props_vbox


func _populate_property_list(
		parent: VBoxContainer,
		class_name_str: StringName,
		controls_map: Dictionary,
		is_mesh: bool) -> void:
	for prop: Dictionary in ClassDB.class_get_property_list(class_name_str, true):
		var usage: int = prop["usage"]
		var prop_name: String = prop["name"]

		if (usage & PROPERTY_USAGE_GROUP) != 0:
			parent.add_child(HSeparator.new())
			var group_label := Label.new()
			group_label.text = prop_name
			group_label.add_theme_font_size_override("font_size", 14)
			group_label.modulate = Color(0.75, 0.92, 0.75)
			parent.add_child(group_label)
			continue

		var prop_type: int = prop["type"]
		if prop_type == TYPE_BOOL:
			_add_bool_row(parent, prop, controls_map, is_mesh)
		elif prop_type == TYPE_INT or prop_type == TYPE_FLOAT:
			_add_numeric_row(parent, prop, controls_map, is_mesh)


func _add_bool_row(
		parent: VBoxContainer,
		prop: Dictionary,
		controls_map: Dictionary,
		is_mesh: bool) -> void:
	var prop_name: String = prop["name"]
	var hbox := HBoxContainer.new()
	parent.add_child(hbox)

	var label := Label.new()
	label.text = prop_name.capitalize()
	label.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	label.add_theme_font_size_override("font_size", 12)
	hbox.add_child(label)

	var check := CheckBox.new()
	hbox.add_child(check)
	controls_map[prop_name] = [check]

	check.toggled.connect(func(pressed: bool) -> void:
		var target: Object = _mesh_settings if is_mesh else _settings
		if _updating_ui or target == null:
			return
		target.set(prop_name, pressed)
		if is_mesh:
			mesh_settings_changed.emit()
		else:
			settings_changed.emit()
	)


func _add_numeric_row(
		parent: VBoxContainer,
		prop: Dictionary,
		controls_map: Dictionary,
		is_mesh: bool) -> void:
	var prop_name: String = prop["name"]
	var is_int: bool = prop["type"] == TYPE_INT
	var hint: int = prop["hint"]
	var hint_string: String = prop["hint_string"]

	var min_val := -2147483647.0
	var max_val := 2147483647.0
	var step_val := 1.0 if is_int else 0.01
	var allow_greater := true
	var has_range := false

	if hint == PROPERTY_HINT_RANGE and not hint_string.is_empty():
		var parts := hint_string.split(",")
		if parts.size() >= 2:
			min_val = parts[0].to_float()
			max_val = parts[1].to_float()
			has_range = true
		if parts.size() >= 3 and parts[2].is_valid_float():
			step_val = parts[2].to_float()
		allow_greater = "or_greater" in parts

	var row_vbox := VBoxContainer.new()
	row_vbox.add_theme_constant_override("separation", 2)
	parent.add_child(row_vbox)

	var top_hbox := HBoxContainer.new()
	row_vbox.add_child(top_hbox)

	var label := Label.new()
	label.text = prop_name.capitalize()
	label.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	label.add_theme_font_size_override("font_size", 12)
	top_hbox.add_child(label)

	var spin := SpinBox.new()
	spin.min_value = min_val
	spin.max_value = max_val
	spin.step = step_val
	spin.rounded = is_int
	spin.allow_greater = allow_greater
	spin.custom_minimum_size = Vector2(95.0, 0.0)
	top_hbox.add_child(spin)

	var linked: Array[Range] = [spin]

	if has_range:
		var slider := HSlider.new()
		slider.min_value = min_val
		slider.max_value = max_val
		slider.step = step_val
		slider.rounded = is_int
		slider.size_flags_horizontal = Control.SIZE_EXPAND_FILL
		slider.scrollable = false
		row_vbox.add_child(slider)
		linked.append(slider)

	controls_map[prop_name] = linked

	for ctrl: Range in linked:
		ctrl.value_changed.connect(func(new_value: float) -> void:
			var target: Object = _mesh_settings if is_mesh else _settings
			if _updating_ui or target == null:
				return
			_updating_ui = true
			for other: Range in linked:
				if other != ctrl:
					other.value = new_value
			_updating_ui = false

			if is_int:
				target.set(prop_name, int(round(new_value)))
			else:
				target.set(prop_name, new_value)

			if is_mesh:
				mesh_settings_changed.emit()
			else:
				settings_changed.emit()
		)


func _ensure_presets_dir() -> void:
	DirAccess.make_dir_recursive_absolute(ProjectSettings.globalize_path("res://presets"))


func _on_save_preset(path: String) -> void:
	if _settings == null:
		return
	var err := ResourceSaver.save(_settings, path)
	if err == OK:
		set_status("Saved preset: %s" % path.get_file())
	else:
		set_status("Save failed (error %d)" % err, true)


func _on_load_preset(path: String) -> void:
	var loaded := ResourceLoader.load(path, "IvySettings", ResourceLoader.CACHE_MODE_IGNORE)
	if loaded is IvySettings:
		var copy := loaded.duplicate() as IvySettings
		set_settings(copy)
		preset_loaded.emit(copy)
		set_status("Loaded preset: %s" % path.get_file())
	else:
		set_status("Failed to load IvySettings from %s" % path.get_file(), true)
