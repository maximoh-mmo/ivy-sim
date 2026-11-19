# Copyright (c) 2026 Max Heinze. All rights reserved.
#
# Licensed for non-commercial use with attribution only.
# Commercial use is strictly prohibited.

extends Node


func _ready() -> void:
	var example := IvyGenerator.new()
	example.generate()
