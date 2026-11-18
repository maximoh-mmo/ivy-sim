#include "ivy_generator.h"

void IvyGenerator::_bind_methods() {
	godot::ClassDB::bind_method(D_METHOD("print_type", "variant"), &IvyGenerator::print_type);
}

void IvyGenerator::print_type(const Variant &p_variant) const {
	print_line(vformat("Type: %d", p_variant.get_type()));
}
