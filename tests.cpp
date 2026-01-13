#include <gtest/gtest.h>
#include <algorithm>
#include "candidate_generation.h"

bool contains_pair(const mags::CandidateSet& cs, int u, int v) {
    const mags::CandidatePair target = (u < v) ? std::make_pair(u, v) : std::make_pair(v, u);
    return std::find(cs.begin(), cs.end(), target) != cs.end();
}

TEST(GreedyCandidateGeneration, DiamondGraphCompleteness) {
    // Diamond Graph
    mags::Graph graph(4);
    graph[0] = {1, 2};
    graph[1] = {0, 3};
    graph[2] = {0, 3};
    graph[3] = {1, 2};

    const auto candidates = generate_candidates(graph);

    // All combinations should be found in this tiny dense graph
    // (0,1), (0,2), (0,3), (1,2), (1,3), (2,3) = 6 pairs
    EXPECT_EQ(candidates.size(), 6);

    // Verify specific 2-hop pairs are found
    EXPECT_TRUE(contains_pair(candidates, 0, 3)) << "Nodes 0 and 3 share neighbors 1 and 2";
    EXPECT_TRUE(contains_pair(candidates, 1, 2)) << "Nodes 1 and 2 share neighbors 0 and 3";

    // 3. Verify 1-hop pairs are found
    EXPECT_TRUE(contains_pair(candidates, 0, 1));
    EXPECT_TRUE(contains_pair(candidates, 0, 2));
    EXPECT_TRUE(contains_pair(candidates, 2, 3));
    EXPECT_TRUE(contains_pair(candidates, 1, 3));
}

TEST(GreedyCandidateGeneration, NoInvalidPairs) {
    mags::Graph graph(3);
    graph[0] = {1};
    graph[1] = {0, 2};
    graph[2] = {1};

    const auto candidates = generate_candidates(graph);

    for (const auto& pair : candidates) {
        EXPECT_LT(pair.first, pair.second) << "Self-loop or reversed pair detected";

        // Ensure IDs are within graph bounds
        EXPECT_GE(pair.first, 0);
        EXPECT_LT(pair.second, 3);
    }
}

TEST(GreedyCandidateGeneration, TriangleNoDuplicates) {
    mags::Graph graph(3);
    graph[0] = {1, 2}; graph[1] = {0, 2}; graph[2] = {0, 1};
    const auto candidates = generate_candidates(graph);
    // Should be exactly 3 pairs: (0,1), (0,2), (1,2)
    EXPECT_EQ(candidates.size(), 3);
}

TEST(GreedyCandidateGeneration, IsolatedNode) {
    mags::Graph graph(5);
    graph[0] = {1}; graph[1] = {0}; // Components 0-1
    // Node 2, 3, 4 are isolated
    const auto candidates = generate_candidates(graph);
    EXPECT_EQ(candidates.size(), 1); // Only (0,1)
}

TEST(GreedyCandidateGeneration, StarGraph) {
    mags::Graph graph(4);
    graph[0] = {1, 2, 3}; graph[1] = {0}; graph[2] = {0}; graph[3] = {0};
    const auto candidates = generate_candidates(graph);
    // 1-hop: (0,1), (0,2), (0,3)
    // 2-hop: (1,2), (1,3), (2,3) -> Total 6
    EXPECT_EQ(candidates.size(), 6);
}