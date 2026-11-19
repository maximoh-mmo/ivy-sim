/*
 * Copyright (c) 2026 Max Heinze. All rights reserved.
 *
 * Licensed for non-commercial use with attribution only.
 * Commercial use is strictly prohibited.
 */

#pragma once

class IvyRandomTests {
public:
    bool run_all();

private:
    bool run_test(const char* name, bool (IvyRandomTests::*test)());

    bool test_reference_sequence();
    bool test_random_int_range();
    bool test_random_int_fixed_range();
    bool test_random_int_full_range();
    bool test_random_int_invalid_range();
    bool test_random_bool();
    bool test_random_float();
    bool test_random_float_range();

    bool expect(bool condition, const char* message);

    int m_passed = 0;
    int m_failed = 0;
};