#pragma once

#include <cstdint>

namespace ivygenerator {

struct IvyMeshConfig {
	int32_t radial_segments = 6; // Cross-section sides (>= 3)
	float radius_scale = 0.5f; // Multiplier from graph thickness to tube radius (> 0)
	float wall_offset_ratio = 0.85f; // Offset along wall_normal as fraction of radius (>= 0)
	float uv_v_scale = 1.0f; // World-length-to-V texture tiling multiplier (> 0)
	bool cap_tips = true; // Close terminal chain tips with a tapered apex

	// Leaf placement
	int32_t leaf_seed = 42; // Deterministic PCG32 seed for leaf placement
	float leaf_density = 4.0f; // Leaves per world unit of stem length (>= 0; 0 disables)
	float leaf_min_thickness = 0.0f; // Skip segments thinner than this (>= 0)
	float leaf_max_thickness = 0.0f; // Skip segments thicker than this (>= 0; 0 = unlimited)
	float leaf_size_min = 0.08f; // Minimum uniform leaf scale (> 0)
	float leaf_size_max = 0.16f; // Maximum uniform leaf scale (>= leaf_size_min)
	float leaf_tilt_radians = 0.5235988f; // Lift angle off wall plane (~30 deg, [0, pi/2])
	float leaf_yaw_variation = 0.5235988f; // Random yaw jitter around stem (~30 deg, >= 0)
	float leaf_roll_variation = 0.3490659f; // Random roll jitter around blade axis (~20 deg, >= 0)
};

} // namespace ivygenerator