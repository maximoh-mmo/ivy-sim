/*
 * Copyright (c) 2026 Max Heinze. All rights reserved.
 *
 * Licensed for non-commercial use with attribution only.
 * Commercial use is strictly prohibited.
 */

#pragma once
#include <cstdint>

// Ivy Random Number Generator
// This is a deterministic random number generator based on the PCG32 algorithm
// It is used to generate random numbers for the simulation in a reproducible manner,
// allowing for consistent results across different runs of the simulation.
// The generator is seeded with a specific value to ensure that the sequence of random numbers
// produced is the same for each run, which is important for debugging and testing purposes.

class IvyRandom {
public:
    // Constructor with optional seed and stream parameters
    explicit IvyRandom(int32_t seed = 42, uint64_t stream = 54);

    // Sets a new seed for the random number generator
	void set_seed(int32_t seed);

    // Sets a new seed and stream for the random number generator
    void set_seed(int32_t seed, uint64_t stream);

	// Generates a random unsigned 32-bit integer
    uint32_t next_u32();

    // Generates a random integer in the range [min, max]
    uint32_t random_int(uint32_t min, uint32_t max);

    // Generate a true or false value with equal probability
    bool random_bool();

    // Generates a random float in the range [min, max]
    float random_float(float min, float max);

    // Generates a random float in the range [0.0, 1.0]
    float random_float();

private:
    // Initializes the random number generator with a given seed
    void initialize(int32_t seed, uint64_t stream);
    
    static uint32_t rotate_right(uint32_t value, uint32_t rotation);

    uint64_t m_seed = 0; // Seed value for the generator
    uint64_t m_state = 0; // Internal state of the generator
    uint64_t m_increment = 0;   // Increment value for the generator
    uint64_t m_stream = 54; // Stream value for the generator, used to create different sequences of random numbers

    static constexpr uint64_t PCG32_MULTIPLIER = 6364136223846793005ULL; // Multiplier for the PCG32 algorithm
};