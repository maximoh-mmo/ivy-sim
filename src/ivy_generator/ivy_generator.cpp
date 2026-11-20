/*
 * Copyright (c) 2026 Max Heinze. All rights reserved.
 *
 * Licensed for non-commercial use with attribution only.
 * Commercial use is strictly prohibited.
 */

/**
 * @file ivy_generator.cpp
 * @brief Implementation of IvyGenerator RefCounted lifecycle and generation routines.
 */

#include "ivy_generator.h"
#include <sstream>

namespace ivygenerator {
using namespace godot;

void IvyGenerator::_bind_methods() {
	godot::ClassDB::bind_method(D_METHOD("generate"), &IvyGenerator::generate);
}
IvyGenerator::IvyGenerator(int32_t seed) : m_seed(seed), m_random(IvyRandom(seed)) {
}

void IvyGenerator::generate() {
	test_cases(); // Run test cases to demonstrate the functionality of IvyGenerator and IvyRandom
}

void IvyGenerator::set_seed(const int32_t seed) {
	m_seed = seed;
	m_random.set_seed(seed);
}

void IvyGenerator::test_cases() {
	// Test case 1: Generate 6 random unsigned 32-bit integers with seed 42 and stream 54
	set_seed(42);
	random_test();
	random_int_test();
	random_bool_test();
	random_float_test();
}

void IvyGenerator::random_test() {
	// Test case 1: Generate 6 random unsigned 32-bit integers with seed 42 and stream 54
	// expected output = [0xA15C02B7, 0x7B47F409, 0xBA1D3330, 0x83D2F293, 0xBFA4784B, 0xCBED606E]
	for (int i = 0; i < 6; ++i) {
		std::ostringstream stream;
		stream << std::uppercase << std::hex << m_random.next_u32();
		std::string output = stream.str();
		print_line(vformat("Random u32: 0x%s", String(output.c_str())));
	}
}

void IvyGenerator::random_int_test() {
	// Test case 2: Generate 20 random integers in the range [10, 20]
	// expected output = [10, 16, 12, 16, 16, 14, 18, 15, 10, 10, 17, 16, 11, 19, 17, 20, 19, 14, 18, 11]

	String random_values;
	for (int i = 0; i < 20; ++i) {
		uint32_t random_value = m_random.random_int(10, 20);
		random_values += vformat("%d, ", random_value);
	}
	print_line(vformat("Random int in range [10, 20]: [%s]", random_values.trim_suffix(", ")));
	print_line(vformat("Random int in range [5, 5]: %d", m_random.random_int(5, 5)));
	print_line(vformat("Random int in range [0, 1]: %d", m_random.random_int(0, 1)));
	print_line(vformat("Random int in range [0, UINT32_MAX]: %u", m_random.random_int(0, UINT32_MAX)));
	print_line(vformat("Random int in range [20, 10]: %d", m_random.random_int(20, 10)));
}

void IvyGenerator::random_bool_test() {
	// Test case 3: Generate 10,000 random boolean values and count the number of true and false values
	// expected output: True count = 4898, False count = 5102

	int true_count = 0;
	int false_count = 0;

	for (int i = 0; i < 10000; ++i) {
		if (m_random.random_bool()) {
			++true_count;
		} else {
			++false_count;
		}
	}

	print_line(vformat("Random bool test: True count = %d, False count = %d", true_count, false_count));
}

void IvyGenerator::random_float_test() {
	// Test case 4: Generate 5 random floats in range [0.0, 1.0) and range [10.0, 20.0)
	String unit_floats;
	String range_floats;
	for (int i = 0; i < 5; ++i) {
		unit_floats += vformat("%.4f, ", m_random.random_float());
		range_floats += vformat("%.4f, ", m_random.random_float(10.0f, 20.0f));
	}
	print_line(vformat("Random float in [0.0, 1.0): [%s]", unit_floats.trim_suffix(", ")));
	print_line(vformat("Random float in [10.0, 20.0): [%s]", range_floats.trim_suffix(", ")));
}
} //namespace ivygenerator