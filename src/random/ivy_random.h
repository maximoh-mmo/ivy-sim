/*
 * Copyright (c) 2026 Max Heinze. All rights reserved.
 *
 * Licensed for non-commercial use with attribution only.
 * Commercial use is strictly prohibited.
 */

#pragma once

#include <cstdint>

/**
 * @file ivy_random.h
 * @brief Deterministic pseudo-random number generator using the PCG32 algorithm.
 */

/**
 * @class IvyRandom
 * @brief Deterministic random number generator based on the PCG32 (PCG-XSH-RR) algorithm.
 *
 * Provides reproducible random number generation for ivy simulation runs,
 * supporting 32-bit unsigned integers, unbiased bounded integers, uniform booleans,
 * and single-precision floating point numbers.
 */
namespace ivygenerator {
class IvyRandom {
public:
	/**
	 * @brief Constructs an IvyRandom generator with a seed and sequence stream.
	 * @param seed Initial seed value for the generator state (default: 42).
	 * @param stream Sequence stream selector (default: 54).
	 */
	explicit IvyRandom(int32_t seed = 42, uint64_t stream = 54);

	/**
	 * @brief Resets the seed using the current stream.
	 * @param seed New seed value.
	 */
	void set_seed(int32_t seed);

	/**
	 * @brief Resets the seed and stream for independent sequence generation.
	 * @param seed New seed value.
	 * @param stream New stream identifier.
	 */
	void set_seed(int32_t seed, uint64_t stream);

	/**
	 * @brief Advances the generator state and produces the next 32-bit unsigned integer.
	 * @return uint32_t Uniformly distributed 32-bit unsigned integer.
	 */
	uint32_t next_u32();

	/**
	 * @brief Generates an unbiased random integer in the inclusive range [min, max].
	 *
	 * Uses Lemire / PCG rejection sampling to eliminate modulo bias.
	 * If min > max, returns min as a safe fallback.
	 *
	 * @param min Inclusive lower bound.
	 * @param max Inclusive upper bound.
	 * @return uint32_t Random integer within [min, max].
	 */
	uint32_t random_int(uint32_t min, uint32_t max);

	/**
	 * @brief Generates a random boolean with 50% probability.
	 * @return bool True or false with equal likelihood.
	 */
	bool random_bool();

	/**
	 * @brief Generates a random float in the range [min, max).
	 * @param min Minimum float value.
	 * @param max Maximum float value.
	 * @return float Uniformly distributed floating point number in [min, max).
	 */
	float random_float(float min, float max);

	/**
	 * @brief Generates a random float in the canonical range [0.0, 1.0).
	 * @return float Uniformly distributed floating point number in [0.0, 1.0).
	 */
	float random_float();

private:
	/**
	 * @brief Internal initialization routine for state and increment calculation.
	 * @param seed Seed value.
	 * @param stream Stream identifier.
	 */
	void initialize(int32_t seed, uint64_t stream);

	/**
	 * @brief Bitwise right-rotation helper for PCG output permutation.
	 * @param value Value to rotate.
	 * @param rotation Number of bits to rotate right.
	 * @return uint32_t Rotated value.
	 */
	static uint32_t rotate_right(uint32_t value, uint32_t rotation);

	uint64_t m_seed = 0; /**< Original seed value passed to the generator. */
	uint64_t m_state = 0; /**< 64-bit internal LCG state. */
	uint64_t m_increment = 0; /**< Stream-dependent odd increment constant. */
	uint64_t m_stream = 54; /**< Stream selector value. */

	static constexpr uint64_t PCG32_MULTIPLIER = 6364136223846793005ULL; /**< Default PCG 64-bit multiplier. */
};
} // namespace ivygenerator