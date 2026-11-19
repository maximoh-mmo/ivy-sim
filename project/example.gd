extends Node


func _ready() -> void:
	var example := IvyGenerator.new()
	add_child(example)
	example.generate()
