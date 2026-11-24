/*
 * Copyright (c) 2026 Max Heinze. All rights reserved.
 *
 * Licensed for non-commercial use with attribution only.
 * Commercial use is strictly prohibited.
 */

#pragma once

#include "ivy_growth_graph.h"

#include <vector>

/**
 * @file ivy_growth_solver.h
 * @brief Algorithmic solver for ivy branch propagation and adhesion.
 */

namespace ivygenerator {
struct IvyWall;
class IvySurfaceSampler;
struct IvyGrowthConfig;
class IvyRandom;

/**
 * @enum GrowthStepResult
 * @brief Result state returned by a single simulation step.
 */
enum class GrowthStepResult {
	ADVANCED, /**< One or more active tips advanced to new valid surface points. */
	FINISHED, /**< All active tips reached boundaries or terminated; simulation complete. */
	ERROR /**< Solver uninitialized or encountered an invalid state. */
};

/**
 * @class IvyGrowthSolver
 * @brief Iterative solver simulating vine propagation across an oriented planar wall surface.
 *
 * Coordinates graph topology, surface boundary clipping, directional momentum,
 * upward tropism, and seeded random wander.
 */
class IvyGrowthSolver {
public:
	/**
	 * @brief Initializes the solver with environment geometry, guidance maps, configuration, and root positions.
	 * @param wall Planar surface representation.
	 * @param surface_sampler Attraction and surface sampler.
	 * @param config Growth parameters and directional weights.
	 * @param random Deterministic random number generator.
	 * @param start_points List of root start coordinates on the wall surface.
	 * @return bool True if all inputs and start points are valid and roots are placed.
	 */
	bool initialize(
			const IvyWall &wall,
			const IvySurfaceSampler &surface_sampler,
			const IvyGrowthConfig &config,
			IvyRandom &random,
			const std::vector<godot::Vector3> &start_points);

	/**
	 * @brief Advances all active tips by one step length across the wall surface.
	 * @return GrowthStepResult ADVANCED if tips grew, FINISHED if all tips ended, ERROR on invalid state.
	 */
	GrowthStepResult step();

	/**
	 * @brief Checks whether the simulation has finished (no active tips remain).
	 * @return bool True if finished or uninitialized.
	 */
	bool is_finished() const;

	/**
	 * @brief Returns a reference to the underlying growth graph containing all generated points.
	 * @return const IvyGrowthGraph& Graph reference.
	 */
	const IvyGrowthGraph &get_graph() const;

private:
	IvyGrowthGraph m_graph;
	std::vector<int32_t> m_active_tips;

	const IvyWall *m_wall = nullptr;
	const IvySurfaceSampler *m_surface_sampler = nullptr;
	const IvyGrowthConfig *m_config = nullptr;
	IvyRandom *m_random = nullptr;

	bool compute_previous_local_direction(
			const IvyPoint &tip,
			const godot::Vector2 &tip_local,
			godot::Vector2 &out_direction) const;

	bool compute_step_local_direction(
			const IvyPoint &tip,
			const godot::Vector2 &tip_local,
			godot::Vector2 &out_direction);

	/**
	 * @brief Computes normalized surface attraction gradient and contour tangent vectors.
	 * @param tip_local Current 2D local position of the active tip.
	 * @param reference_direction Base heading used to resolve contour tangent orientation.
	 * @param out_attraction_direction Output unit vector pointing along gradient ascent.
	 * @param out_contour_direction Output unit vector pointing along contour tangent.
	 */
	void compute_attraction_vectors(
			const godot::Vector2 &tip_local,
			const godot::Vector2 &reference_direction,
			godot::Vector2 &out_attraction_direction,
			godot::Vector2 &out_contour_direction) const;

	/**
	 * @brief Attempts to spawn a secondary branch tip from a parent point.
	 * @param parent_index Index of the parent IvyPoint in the graph.
	 * @param parent_local 2D local position on the wall of the parent tip.
	 * @param main_direction_local Forward growth direction vector of the main branch.
	 * @param next_active_tips Vector accumulating active tip indices for the subsequent step.
	 * @return True if branch handling completed successfully, false on allocation/graph failure.
	 */
	bool try_spawn_branch(
			int32_t parent_index,
			const godot::Vector2 &parent_local,
			const godot::Vector2 &main_direction_local,
			std::vector<int32_t> &next_active_tips);

	/**
	 * @brief Updates stem radii across the graph based on active tip support and branch termination.
	 * @details Non-active internal segments accumulate downstream tip counts to simulate vascular thickening
	 * (Leonardo's da Vinci / pipe model rule), while segments with zero active downstream tips wither.
	 * @param active_tips List of tip indices that remain active in the simulation.
	 * @return True if thickness updates succeeded, false on graph indexing error.
	 */
	bool update_thicknesses(const std::vector<int32_t> &active_tips);
};

} // namespace ivygenerator
