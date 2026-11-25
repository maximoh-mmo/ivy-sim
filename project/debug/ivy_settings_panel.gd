# Copyright (c) 2026 Max Heinze. All rights reserved.
#
# Licensed for non-commercial use with attribution only.
# Commercial use is strictly prohibited.

class_name IvySettingsPanel
extends CanvasLayer

signal settings_changed
signal reset_requested
signal preset_loaded(new_settings: IvySettings)

var _settings: IvySettings
var _controls: Dictionary = {} # property_name -> Array[Range] (slider + spinbox)
var _updating_ui := false

var _status_label: Label
var _save_dialog: FileDialog
var _load_dialog: FileDialog


func setup(settings: IvySettings) -> void:
	_settings = settings
	_build_ui()
	sync_from_settings()


func set_settings(settings: IvySettings) -> void:
	_settings = settings
	sync_from_settings()


func set_status(text: String, is_error: bool = false) -> void:
	if _status_label == null:
		return
	_status_label.text = text
	_status_label.modulate = Color(1.0, 0.45, 0.45) if is_error else Color(0.85, 0.95, 0.85)


func sync_from_settings() -> void:
	if _settings == null:
		return
	_updating_ui = true
	for prop_name: String in _controls:
		var val: float = float(_settings.get(prop_name))
		for ctrl: Range in _controls[prop_name]:
			ctrl.value = val
	_updating_ui = false


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

	# Scrollable auto-generated property list from IvySettings ClassDB metadata
	var scroll := ScrollContainer.new()
	scroll.size_flags_vertical = Control.SIZE_EXPAND_FILL
	scroll.horizontal_scroll_mode = ScrollContainer.SCROLL_MODE_DISABLED
	root_vbox.add_child(scroll)

	var props_vbox := VBoxContainer.new()
	props_vbox.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	props_vbox.add_theme_constant_override("separation", 6)
	scroll.add_child(props_vbox)

	for prop: Dictionary in ClassDB.class_get_property_list(&"IvySettings", true):
		var usage: int = prop["usage"]
		var prop_name: String = prop["name"]

		if (usage & PROPERTY_USAGE_GROUP) != 0:
			var group_sep := HSeparator.new()
			props_vbox.add_child(group_sep)
			var group_label := Label.new()
			group_label.text = prop_name
			group_label.add_theme_font_size_override("font_size", 14)
			group_label.modulate = Color(0.75, 0.92, 0.75)
			props_vbox.add_child(group_label)
			continue

		var prop_type: int = prop["type"]
		if prop_type != TYPE_INT and prop_type != TYPE_FLOAT:
			continue

		_add_numeric_row(props_vbox, prop)

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


func _add_numeric_row(parent: VBoxContainer, prop: Dictionary) -> void:
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

	_controls[prop_name] = linked

	for ctrl: Range in linked:
		ctrl.value_changed.connect(func(new_value: float) -> void:
			if _updating_ui or _settings == null:
				return
			_updating_ui = true
			for other: Range in linked:
				if other != ctrl:
					other.value = new_value
			_updating_ui = false

			if is_int:
				_settings.set(prop_name, int(round(new_value)))
			else:
				_settings.set(prop_name, new_value)
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
