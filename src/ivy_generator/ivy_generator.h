#pragma once

#include "godot_cpp/classes/ref_counted.hpp"
#include "random/ivy_random.h"

using namespace godot;

class IvyGenerator : public RefCounted {
	GDCLASS(IvyGenerator, RefCounted)

protected:
	static void _bind_methods();

public:
	IvyGenerator(const int32_t seed = 42);
	~IvyGenerator() override = default;

	void generate();
	void set_seed(const int32_t seed);

private:
	int32_t m_seed;
	IvyRandom m_random;


	// Test cases for the IvyGenerator class
	void test_cases();
	void random_test();
	void random_int_test();
	void random_bool_test();
	void random_float_test();
};
