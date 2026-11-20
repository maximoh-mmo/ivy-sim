/*
 * Copyright (c) 2026 Max Heinze. All rights reserved.
 *
 * Licensed for non-commercial use with attribution only.
 * Commercial use is strictly prohibited.
 */

/**
 * @file ivy_growth_graph_tests.cpp
 * @brief Implementation of IvyGrowthGraph unit test cases.
 */

#include "ivy_growth_graph_tests.h"

#include <iostream>

namespace ivygenerator {

void IvyGrowthGraphTests::expect(
    const char* test_name,
    bool condition
) {
    if (condition) {
        tests_passed++;
        std::cout << "PASS: " << test_name << '\n';
    } else {
        tests_failed++;
        std::cout << "FAIL: " << test_name << '\n';
    }
}

bool IvyGrowthGraphTests::run_all() {
    tests_passed = 0;
    tests_failed = 0;

    std::cout << "\n=== IvyGrowthGraph Unit Tests ===\n\n";

    test_add_root();
    test_add_child();
    test_branching();
    test_invalid_negative_parent();
    test_invalid_parent_index();
    test_get_point();
    test_clear();

    std::cout << "\n=== IvyGrowthGraph Test Summary ===\n";
    std::cout << "Passed: " << tests_passed << '\n';
    std::cout << "Failed: " << tests_failed << '\n';
    return tests_failed == 0;
}

void IvyGrowthGraphTests::test_add_root() {
    ivygenerator::IvyGrowthGraph graph;

    ivygenerator::IvyPoint point;
    point.position = godot::Vector3(1.0f, 2.0f, 3.0f);
    point.thickness = 0.5f;
    point.parent_index = -1;

    const int32_t index = graph.add_point(point);

    expect("Add root point", index == 0);
    expect("Root increases point count", graph.get_point_count() == 1);
}

void IvyGrowthGraphTests::test_add_child() {
    ivygenerator::IvyGrowthGraph graph;

    ivygenerator::IvyPoint root;
    root.parent_index = -1;

    graph.add_point(root);

    ivygenerator::IvyPoint child;
    child.parent_index = 0;

    const int32_t index = graph.add_point(child);

    expect("Add child point", index == 1);
    expect("Child increases point count", graph.get_point_count() == 2);
}

void IvyGrowthGraphTests::test_branching() {
    ivygenerator::IvyGrowthGraph graph;

    ivygenerator::IvyPoint root;
    root.parent_index = -1;
    graph.add_point(root);

    ivygenerator::IvyPoint child_a;
    child_a.parent_index = 0;

    ivygenerator::IvyPoint child_b;
    child_b.parent_index = 0;

    const int32_t index_a = graph.add_point(child_a);
    const int32_t index_b = graph.add_point(child_b);

    expect("First branch child added", index_a == 1);
    expect("Second branch child added", index_b == 2);
    expect("Both children share parent",
        graph.get_point(1)->parent_index == 0 &&
        graph.get_point(2)->parent_index == 0);
}

void IvyGrowthGraphTests::test_invalid_negative_parent() {
    ivygenerator::IvyGrowthGraph graph;

    ivygenerator::IvyPoint point;
    point.parent_index = -2;

    const int32_t index = graph.add_point(point);

    expect("Reject negative parent index", index == -1);
    expect("Invalid point not added", graph.get_point_count() == 0);
}

void IvyGrowthGraphTests::test_invalid_parent_index() {
    ivygenerator::IvyGrowthGraph graph;

    ivygenerator::IvyPoint point;
    point.parent_index = 5;

    const int32_t index = graph.add_point(point);

    expect("Reject nonexistent parent", index == -1);
    expect("Invalid parent does not change graph", graph.get_point_count() == 0);
}

void IvyGrowthGraphTests::test_get_point() {
    ivygenerator::IvyGrowthGraph graph;

    ivygenerator::IvyPoint point;
    point.position = godot::Vector3(1.0f, 2.0f, 3.0f);
    point.thickness = 0.75f;
    point.parent_index = -1;

    graph.add_point(point);

    const ivygenerator::IvyPoint* retrieved = graph.get_point(0);

    expect("Retrieve valid point", retrieved != nullptr);
    expect("Retrieved position matches",
        retrieved != nullptr &&
        retrieved->position == godot::Vector3(1.0f, 2.0f, 3.0f));
    expect("Retrieved thickness matches",
        retrieved != nullptr && retrieved->thickness == 0.75f);
    expect("Reject negative retrieval index", graph.get_point(-1) == nullptr);
    expect("Reject out-of-range retrieval index", graph.get_point(1) == nullptr);
}

void IvyGrowthGraphTests::test_clear() {
    ivygenerator::IvyGrowthGraph graph;

    ivygenerator::IvyPoint root;
    root.parent_index = -1;
    graph.add_point(root);

    graph.clear();

    expect("Clear removes all points", graph.get_point_count() == 0);
    expect("Cleared graph has no accessible points", graph.get_point(0) == nullptr);
}

} // namespace ivygenerator
