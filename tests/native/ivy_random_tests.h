/*
 * Copyright (c) 2026 Max Heinze. All rights reserved.
 *
 * Licensed for non-commercial use with attribution only.
 * Commercial use is strictly prohibited.
 */

#pragma once

/**
 * @file ivy_random_tests.h
 * @brief Unit test suite for IvyRandom PCG32 implementation.
 */

/**
 * @class IvyRandomTests
 * @brief Test suite verifying determinism, bounds, and probability distribution of IvyRandom.
 */

namespace ivygenerator {

class IvyRandomTests {
public:
	/**
	 * @brief Executes all IvyRandom unit tests and prints results to stdout.
	 * @return bool True if all tests passed.
	 */
	bool run_all();

private:
	/**
	 * @brief Helper to execute and format an individual member test function.
	 * @param name Test display name.
	 * @param test Member function pointer to the test method.
	 * @return bool True if test succeeded.
	 */
	bool run_test(const char *name, bool (IvyRandomTests::*test)());

	bool test_reference_sequence();
	bool test_random_int_range();
	bool test_random_int_fixed_range();
	bool test_random_int_full_range();
	bool test_random_int_invalid_range();
	bool test_random_bool();
	bool test_random_float();
	bool test_random_float_range();

	/**
	 * @brief Assertion helper tracking passed and failed test count.
	 * @param condition Condition expression to evaluate.
	 * @param message Failure explanation printed when condition is false.
	 * @return bool True if condition was true.
	 */
	bool expect(bool condition, const char *message);

	int m_passed = 0; /**< Count of passing assertions. */
	int m_failed = 0; /**< Count of failing assertions. */
};
} // namespace ivygenerator