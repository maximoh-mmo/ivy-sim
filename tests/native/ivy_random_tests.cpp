/*
 * Copyright (c) 2026 Max Heinze. All rights reserved.
 *
 * Licensed for non-commercial use with attribution only.
 * Commercial use is strictly prohibited.
 */

/**
 * @file ivy_random_tests.cpp
 * @brief Implementation of IvyRandom test assertions and test runner execution.
 */

#include "ivy_random_tests.h"
#include "random/ivy_random.h"

#include <iostream>
#include <cstdint>
#include <cmath>

namespace ivygenerator {

bool IvyRandomTests::expect(bool condition, const char* message) {
    if (!condition) {
        std::cout << "    FAILED: " << message << '\n';
    }

    return condition;
}

bool IvyRandomTests::run_test(
    const char* name,
    bool (IvyRandomTests::*test)()
) {
    std::cout << "Running: " << name << '\n';

    const bool result = (this->*test)();

    if (result) {
        ++m_passed;
        std::cout << "    PASS\n";
    } else {
        ++m_failed;
        std::cout << "    FAIL\n";
    }

    return result;
}

bool IvyRandomTests::run_all() {
    m_passed = 0;
    m_failed = 0;

    std::cout << "\n=== IvyRandom Unit Tests ===\n\n";

    run_test("PCG32 reference sequence",
        &IvyRandomTests::test_reference_sequence);

    run_test("Random integer range",
        &IvyRandomTests::test_random_int_range);

    run_test("Random integer fixed range",
        &IvyRandomTests::test_random_int_fixed_range);

    run_test("Random integer full range",
        &IvyRandomTests::test_random_int_full_range);

    run_test("Random integer invalid range",
        &IvyRandomTests::test_random_int_invalid_range);

    run_test("Random boolean",
        &IvyRandomTests::test_random_bool);

    run_test("Random float",
        &IvyRandomTests::test_random_float);

    run_test("Random float range",
        &IvyRandomTests::test_random_float_range);

    std::cout << "\n=== Test Summary ===\n";
    std::cout << "Passed: " << m_passed << '\n';
    std::cout << "Failed: " << m_failed << '\n';

    return m_failed == 0;
}

bool IvyRandomTests::test_reference_sequence() {
    IvyRandom rng(42, 54);

    const uint32_t expected[] = {
        0xA15C02B7,
        0x7B47F409,
        0xBA1D3330,
        0x83D2F293,
        0xBFA4784B,
        0xCBED606E
    };

    for (uint32_t value : expected) {
        if (!expect(rng.next_u32() == value,
                    "PCG32 output does not match reference")) {
            return false;
        }
    }

    return true;
}

bool IvyRandomTests::test_random_int_range() {
    IvyRandom rng(42);

    for (int i = 0; i < 10000; ++i) {
        const uint32_t value = rng.random_int(10, 20);

        if (!expect(value >= 10 && value <= 20,
                    "Value outside requested range")) {
            return false;
        }
    }

    return true;
}

bool IvyRandomTests::test_random_int_fixed_range() {
    IvyRandom rng(42);

    for (int i = 0; i < 100; ++i) {
        if (!expect(rng.random_int(5, 5) == 5,
                    "Fixed range should always return 5")) {
            return false;
        }
    }

    return true;
}

bool IvyRandomTests::test_random_int_full_range() {
    IvyRandom rng_a(42);
    IvyRandom rng_b(42);

    const uint32_t actual =
        rng_a.random_int(0, UINT32_MAX);

    const uint32_t expected =
        rng_b.next_u32();

    return expect(actual == expected,
                  "Full range should return next_u32()");
}

bool IvyRandomTests::test_random_int_invalid_range() {
    IvyRandom rng(42);

    return expect(rng.random_int(20, 10) == 20,
                  "Invalid range should return min");
}

bool IvyRandomTests::test_random_bool() {
    IvyRandom rng_a(42);
    IvyRandom rng_b(42);

    bool found_true = false;
    bool found_false = false;

    for (int i = 0; i < 1000; ++i) {
        const bool a = rng_a.random_bool();
        const bool b = rng_b.random_bool();

        if (!expect(a == b, "Boolean sequences differ")) {
            return false;
        }

        if (a) {
            found_true = true;
        } else {
            found_false = true;
        }
    }

    return expect(found_true && found_false,
                  "Expected both boolean outcomes");
}

bool IvyRandomTests::test_random_float() {
    IvyRandom rng_a(42);
    IvyRandom rng_b(42);
    float min_value = 1.0f;
    float max_value = 0.0f;

    for (int i = 0; i < 10000; ++i) {
        const float a = rng_a.random_float();
        const float b = rng_b.random_float();

        if (!expect(std::isfinite(a), "Float is not finite")) {
            return false;
        }

        if (!expect(a >= 0.0f && a < 1.0f,
                    "Float outside [0, 1)")) {
            return false;
        }

        if (a < min_value) {
            min_value = a;
        }

        if (a > max_value) {
            max_value = a;
        }

        if (!expect(a == b, "Float sequences differ")) {
            return false;
        }
    }
    
    return expect(min_value < max_value,
              "Float generator produced no variation");
}

bool IvyRandomTests::test_random_float_range() {
    IvyRandom rng(42);

    for (int i = 0; i < 10000; ++i) {
        const float val = rng.random_float(1.5f, 5.5f);
        if (!expect(val >= 1.5f && val < 5.5f, "Float outside requested range [1.5, 5.5)")) {
            return false;
        }
    }

    if (!expect(rng.random_float(3.0f, 3.0f) == 3.0f, "Fixed range should return min")) {
        return false;
    }

    if (!expect(rng.random_float(5.0f, 2.0f) == 5.0f, "Invalid range should return min")) {
        return false;
    }

    return true;
}

} // namespace ivygenerator

