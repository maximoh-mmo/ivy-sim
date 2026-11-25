/*
 * Copyright (c) 2026 Max Heinze. All rights reserved.
 *
 * Licensed for non-commercial use with attribution only.
 * Commercial use is strictly prohibited.
 */

#pragma once

#include "ivy_growth_graph.h"

#include <godot_cpp/variant/vector2.hpp>
#include <godot_cpp/variant/vector3.hpp>

#include <cstdint>
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
	 * @param target_points Optional list of target waypoint coordinates on the wall surface.
	 * @return bool True if all inputs, start points, and target points are valid and roots are placed.
	 */
	bool initialize(
			const IvyWall &wall,
			const IvySurfaceSampler &surface_sampler,
			const IvyGrowthConfig &config,
			IvyRandom &random,
			const std::vector<godot::Vector3> &start_points,
			const std::vector<godot::Vector3> &target_points = {});

	/**
	 * @brief Advances all active tips by one step length across the wall surface.
	 * @return GrowthStepResult ADVANCED if tips grew, FINISHED if all tips ended, ERROR on invalid state.
	 */
	GrowthStepResult step();

	/**
	 * @brief Connects any still-unsatisfied target waypoints by growing deterministic
	 * lateral tendrils from the closest existing graph points until every target is reached.
	 * @return bool True if all targets are reached (or no targets exist), false if uninitialized
	 * or if max_points capacity is exhausted before reaching all targets.
	 */
	bool connect_unreached_targets();

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

	/**
	 * @brief Returns the number of configured target waypoints.
	 * @return int32_t Number of target waypoints.
	 */
	int32_t get_target_count() const;

	/**
	 * @brief Checks whether the target waypoint at target_index has been reached.
	 * @param target_index Zero-based target index.
	 * @return bool True if index is valid and target was reached within acceptance radius.
	 */
	bool is_target_reached(int32_t target_index) const;

	/**
	 * @brief Returns the total number of target waypoints reached so far.
	 * @return int32_t Count of satisfied targets.
	 */
	int32_t get_reached_target_count() const;

private:
	IvyGrowthGraph m_graph;
	std::vector<int32_t> m_active_tips;
	std::vector<int32_t> m_steps_since_branch;

	// Spacial Grid
	std::vector<godot::Vector2> m_local_positions;
	std::vector<int32_t> m_grid_heads;
	std::vector<int32_t> m_grid_next;
	int32_t m_grid_cols = 0;
	int32_t m_grid_rows = 0;
	float m_grid_cell_size = 0.0f;

	// Target waypoints
	std::vector<godot::Vector2> m_target_points_local;
	std::vector<uint8_t> m_target_reached;

	const IvyWall *m_wall = nullptr;
	const IvySurfaceSampler *m_surface_sampler = nullptr;
	const IvyGrowthConfig *m_config = nullptr;
	IvyRandom *m_random = nullptr;

	bool compute_previous_local_direction(
			const IvyPoint &tip,
			const godot::Vector2 &tip_local,
			godot::Vector2 &out_direction) const;

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
			std::vector<int32_t> &next_active_tips,
			bool &out_branched);

	/**
	 * @brief Updates stem radii across the graph based on active tip support and branch termination.
	 * @details Non-active internal segments accumulate downstream tip counts to simulate vascular thickening
	 * (Leonardo's da Vinci / pipe model rule), while segments with zero active downstream tips wither.
	 * @param active_tips List of tip indices that remain active in the simulation.
	 * @return True if thickness updates succeeded, false on graph indexing error.
	 */
	bool update_thicknesses(const std::vector<int32_t> &active_tips);

	/**
	 * @brief Computes the combined local growth direction for a tip.
	 * @param tip_index Graph index of the advancing tip.
	 * @param tip The IvyPoint data of the advancing tip.
	 * @param tip_local 2D local position on the wall surface.
	 * @param out_direction Output unit vector for the step direction.
	 * @return True if direction evaluation succeeded, false on error.
	 */
	bool compute_step_local_direction(
			int32_t tip_index,
			const IvyPoint &tip,
			const godot::Vector2 &tip_local,
			godot::Vector2 &out_direction);

	/**
	 * @brief Inserts a point into the uniform spatial hash grid.
	 * @param point_index Graph index of the point.
	 * @param local_pos 2D local position on the wall.
	 */
	void register_point_in_grid(
			int32_t point_index,
			const godot::Vector2 &local_pos);

	/**
	 * @brief Checks if candidate_index belongs to the recent ancestor chain or sibling of tip_index.
	 * @param tip_index Graph index of the active tip.
	 * @param candidate_index Graph index of a neighboring point.
	 * @param max_hops Maximum parent hops to traverse upward.
	 * @return True if candidate is closely related in lineage, false otherwise.
	 */
	bool is_recent_lineage(
			int32_t tip_index,
			int32_t candidate_index,
			int32_t max_hops) const;

	/**
	 * @brief Computes a normalized repulsive vector steering away from nearby unrelated points.
	 * @param tip_index Graph index of the active tip.
	 * @param tip_local 2D local position of the tip on the wall.
	 * @return Repulsive unit vector (or zero vector if uncrowded).
	 */
	godot::Vector2 compute_avoidance_vector(
			int32_t tip_index,
			const godot::Vector2 &tip_local) const;

	/**
	 * @brief Counts nearby non-ancestor points within the crowding radius.
	 * @param tip_index Graph index of the advancing tip.
	 * @param query_local Proposed 2D position for the next step.
	 * @return Number of neighboring points within crowding_radius.
	 */
	int32_t count_crowding_neighbors(
			int32_t tip_index,
			const godot::Vector2 &query_local) const;

	/**
	 * @brief Computes a soft steering vector toward the nearest unsatisfied target waypoint.
	 * @param tip_local 2D local position of the active tip on the wall.
	 * @return Steering vector weighted by influence falloff (or zero vector if none in range).
	 */
	godot::Vector2 compute_target_vector(const godot::Vector2 &tip_local) const;

	/**
	 * @brief Marks any unsatisfied target waypoints within target_acceptance_radius of the segment as reached.
	 * @param segment_start 2D local start position of the newly added growth segment.
	 * @param segment_end 2D local end position of the newly added growth segment.
	 */
	void update_reached_targets(
			const godot::Vector2 &segment_start,
			const godot::Vector2 &segment_end);
};

} // namespace ivygenerator
