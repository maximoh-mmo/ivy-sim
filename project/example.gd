extends Node


func _ready() -> void:
	var example := IvyGenerator.new()
	example.print_type(example)
