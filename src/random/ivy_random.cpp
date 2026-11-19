/*
 * Copyright (c) 2026 Max Heinze. All rights reserved.
 *
 * Licensed for non-commercial use with attribution only.
 * Commercial use is strictly prohibited.
 */

#include "ivy_random.h"

IvyRandom::IvyRandom(int32_t seed, uint64_t stream) {
    initialize(seed, stream);
}

uint32_t IvyRandom::random_int(uint32_t min, uint32_t max) {
	// Invalid range: return min as a safe fallback.
    if (min > max) {
        return min;
    }

    // Calculate the range using 64 bits to avoid overflow.
    const uint64_t range = static_cast<uint64_t>(max) - min + 1;

    // Special case: the entire uint32_t range.
    if (range == (uint64_t{1} << 32)) {
        return next_u32();
    }

    const uint32_t bound = static_cast<uint32_t>(range);

    // Calculate the rejection threshold.
    const uint32_t threshold = static_cast<uint32_t>(-bound) % bound;

    // Reject values below the threshold to eliminate modulo bias.
    uint32_t value;
    do {
        value = next_u32();
    } while (value < threshold);

    return min + (value % bound);
}

bool IvyRandom::random_bool() {
    return (next_u32() & 1u) != 0;
}

float IvyRandom::random_float(float min, float max) {
    if (min >= max) {
        return min;
    }
    return min + random_float() * (max - min);
}

float IvyRandom::random_float() {
    return static_cast<float>(next_u32() >> 8) * (1.0f / 16777216.0f);
}

void IvyRandom::set_seed(int32_t seed) {
    initialize(seed, m_stream); // Use the existing stream value when setting a new seed
}

void IvyRandom::set_seed(int32_t seed, uint64_t stream) {
    initialize(seed, stream); // Use the provided stream value when setting a new seed
}

uint32_t IvyRandom::next_u32() {
    // Save the current state before advancing
    const uint64_t old_state = m_state;

    // Advance the state
    m_state = old_state * PCG32_MULTIPLIER + m_increment;

    // Calculate the output value based on the old state
    const uint32_t xorshifted = static_cast<uint32_t>(((old_state >> 18u) ^ old_state) >> 27u);

    const uint32_t rotation = static_cast<uint32_t>(old_state >> 59u);

    // Rotate the xorshifted value to produce the final output
    return rotate_right(xorshifted, rotation);
}

void IvyRandom::initialize(int32_t seed, uint64_t stream) {
    m_seed = static_cast<uint64_t>(static_cast<int64_t>(seed));
    m_stream = stream;
    m_increment = (stream << 1u) | 1u;

    m_state = 0;
    next_u32(); // Advance the state to avoid initial low-quality output
    
    m_state += seed;
    next_u32(); // Advance the state again after seeding
}

uint32_t IvyRandom::rotate_right(uint32_t value, uint32_t rotation) {
    return (value >> rotation) | (value << ((-rotation) & 31));
}