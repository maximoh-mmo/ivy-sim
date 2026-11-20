/*
 * Copyright (c) 2026 Max Heinze. All rights reserved.
 *
 * Licensed for non-commercial use with attribution only.
 * Commercial use is strictly prohibited.
 */

#pragma once

#include "godot_cpp/classes/ref_counted.hpp"
#include "random/ivy_random.h"

/**
 * @file ivy_generator.h
 * @brief Core entry point class for the Ivy simulation GDExtension.
 */

/**
 * @class IvyGenerator
 * @brief Godot RefCounted node providing high-level ivy simulation generation and configuration.
 *
 * Exposes methods to GDScript for initiating growth simulation runs, managing random seeds,
 * and coordinating simulation subsystems.
 */

namespace ivygenerator {
	
	class IvyGenerator : public godot::RefCounted {
		GDCLASS(IvyGenerator, godot::RefCounted)

	protected:
		/**
		 * @brief Godot ClassDB method registration hook.
		 */
		static void _bind_methods();

	public:
		/**
		 * @brief Constructs an IvyGenerator with a deterministic starting seed.
		 * @param seed Initial RNG seed (default: 42).
		 */
		IvyGenerator(const int32_t seed = 42);

		/**
		 * @brief Default virtual destructor.
		 */
		~IvyGenerator() override = default;

		/**
		 * @brief Executes the ivy generation routine and test suite verification.
		 */
		void generate();

		/**
		 * @brief Updates the RNG seed used for procedural ivy generation.
		 * @param seed New random seed value.
		 */
		void set_seed(const int32_t seed);

	private:
		int32_t m_seed;      /**< Current simulation seed. */
		IvyRandom m_random;  /**< Deterministic PCG32 random generator instance. */

		/**
		 * @brief Internal verification routines demonstrating generator output.
		 */
		void test_cases();

		/**
		 * @brief Verifies raw 32-bit unsigned integer generation.
		 */
		void random_test();

		/**
		 * @brief Verifies bounded integer generation.
		 */
		void random_int_test();

		/**
		 * @brief Verifies boolean probability distribution.
		 */
		void random_bool_test();

		/**
		 * @brief Verifies canonical and bounded float generation.
		 */
		void random_float_test();
	};
} // namespace ivygenerator