#include <gtest/gtest.h>
#include <algorithm>
#include "candidate_generation.h"

int count_signature_matches(const int u, const int v, const mags::cg::SignatureMatrix& sigs) {
    int matches = 0;
    for (int h = 0; h < mags::cg::H_FUNCS; ++h) {
        if (sigs.at(u).at(h) == sigs.at(v).at(h)) matches++;
    }
    return matches;
}

TEST(ComputeMinHashes, IdenticalNeighbors) {
    // Graph with 6 nodes, node 0 and 1 has the same neighbors, and nodes 3 and 4 have the same neighbors
    const mags::Graph graph = {
        {2, 3},
        {2, 3},
        {0, 1},
        {0, 1}
    };

    const size_t n = graph.size();
    mags::cg::SignatureMatrix signatures(n, std::vector<int>(mags::cg::H_FUNCS));

    mags::cg::SEED = 2333;

    mags::cg::compute_minhashes(graph, signatures);

    for (int h = 0; h < mags::cg::H_FUNCS; ++h) {
        EXPECT_EQ(signatures.at(0).at(h), signatures.at(1).at(h));
        EXPECT_EQ(signatures.at(2).at(h), signatures.at(3).at(h));
    }
}

TEST(ComputeMinHashes, DisjointNeighbors) {
    // Graph with 6 nodes, node 0 and 1 has the same neighbors, and nodes 3 and 4 have the same neighbors
    const mags::Graph graph = {
        {2, 3},
        {2, 3},
        {0, 1},
        {0, 1},
        {4}
    };
    const size_t n = graph.size();

    mags::cg::SignatureMatrix signatures(n, std::vector<int>(mags::cg::H_FUNCS));

    mags::cg::SEED = 2333;
    mags::cg::compute_minhashes(graph, signatures);

    EXPECT_EQ(count_signature_matches(0, 1, signatures), mags::cg::H_FUNCS);
    EXPECT_EQ(count_signature_matches(0, 4, signatures), 0);
}

TEST(ComputeMinHashes, IsolatedNode) {
    // Graph with 6 nodes, node 2 is an isolated node and should remain unvisited (-1)
    const mags::Graph graph = {
        {0, 1},
        {0, 1},
        {}
    };
    const size_t n = graph.size();

    mags::cg::SignatureMatrix signatures(n, std::vector<int>(mags::cg::H_FUNCS));

    mags::cg::SEED = 2333;
    mags::cg::compute_minhashes(graph, signatures);

    for (int h = 0; h < mags::cg::H_FUNCS; ++h) {
        EXPECT_EQ(signatures.at(2).at(h), -1);
    }
}

TEST(ComputeMinHashes, SmallestGraph) {
    const mags::Graph graph = {
        {0}
    };
    const size_t n = graph.size();

    mags::cg::SignatureMatrix signatures(n, std::vector<int>(mags::cg::H_FUNCS));

    mags::cg::SEED = 2333;
    mags::cg::compute_minhashes(graph, signatures);

    for (int h = 0; h < mags::cg::H_FUNCS; ++h) {
        EXPECT_EQ(signatures.at(0).at(h), 0);
    }
}

TEST(ComputeMinHashes, SeedConsistency) {
    const mags::Graph graph = {
        {1, 2},
        {0, 1},
        {1}
    };
    const size_t n = graph.size();

    mags::cg::SignatureMatrix sig_seed_0_run_a(n, std::vector<int>(mags::cg::H_FUNCS));
    mags::cg::SignatureMatrix sig_seed_0_run_b(n, std::vector<int>(mags::cg::H_FUNCS));
    mags::cg::SignatureMatrix sig_seed_1(n, std::vector<int>(mags::cg::H_FUNCS));

    mags::cg::SEED = 0;
    mags::cg::compute_minhashes(graph, sig_seed_0_run_a);
    mags::cg::compute_minhashes(graph, sig_seed_0_run_b);

    mags::cg::SEED = 1;
    mags::cg::compute_minhashes(graph, sig_seed_1);

    EXPECT_EQ(sig_seed_0_run_a, sig_seed_0_run_b);
    EXPECT_NE(sig_seed_0_run_a, sig_seed_1);
}

TEST(ComputeMinHashes, SelfLoopChangesSimilarity) {
    const mags::Graph g_no_self_loop = {
        {2},
        {2},
        {0, 1}
    };
    const size_t n1 = g_no_self_loop.size();

    const mags::Graph g_with_self_loop = {
        {2, 0},
        {2, 1},
        {0, 1}
    };
    const size_t n2 = g_with_self_loop.size();

    mags::cg::SignatureMatrix sig1(n1, std::vector<int>(mags::cg::H_FUNCS));
    mags::cg::SignatureMatrix sig2(n2, std::vector<int>(mags::cg::H_FUNCS));

    mags::cg::SEED = 2333;
    mags::cg::compute_minhashes(g_no_self_loop, sig1);
    mags::cg::compute_minhashes(g_with_self_loop, sig2);

    EXPECT_NE(sig1.at(1), sig2.at(1));
    EXPECT_NE(count_signature_matches(0, 1, sig1), count_signature_matches(0, 1, sig2));
}

TEST(ComputeMinHashes, DiamonGraphCompleteness) {
    const mags::Graph graph = {
        {1, 2},
        {0, 3},
        {0, 3},
        {1, 2}
    };
    const size_t n = graph.size();

    mags::cg::SignatureMatrix signatures(n, std::vector<int>(mags::cg::H_FUNCS));
    mags::cg::SEED = 2333;
    mags::cg::compute_minhashes(graph, signatures);

    EXPECT_EQ(count_signature_matches(0, 3, signatures), mags::cg::H_FUNCS);
}

bool contains_pair(const mags::cg::CandidatePairSet& cs, int u, int v) {
    // Normalizing a pair to u < v to match Algorithm 3 requirements
    const std::pair<mags::NodeID, mags::NodeID> target = (u < v) ?
    std::make_pair(u, v) : std::make_pair(v, u);

    return cs.contains(target);
}

TEST(CandidateGeneration, DiamondGraphCompleteness) {
    mags::Graph graph(4);
    graph[0] = {1, 2}; graph[1] = {0, 3}; graph[2] = {0, 3}; graph[3] = {1, 2};

    // Using k=30 ensures we keep all candidates for this small graph
    const auto candidates = mags::cg::generate_candidates(graph, 30);

    // Total unique pairs in 4 nodes = 6
    // Mags identifies these through 1-hop and 2-hop neighbor sampling
    EXPECT_EQ(candidates.size(), 6);
    EXPECT_TRUE(contains_pair(candidates, 0, 3));
    EXPECT_TRUE(contains_pair(candidates, 1, 2));
}

TEST(CandidateGeneration, NoInvalidPairs) {
    mags::Graph graph(3);
    graph[0] = {1}; graph[1] = {0, 2}; graph[2] = {1};

    const auto candidates = mags::cg::generate_candidates(graph, 10);

    for (const auto& [fst, snd] : candidates) {
        // Enforce u < v normalization
        EXPECT_LT(fst, snd);
        EXPECT_GE(fst, 0);
        EXPECT_LT(snd, 3);
    }
}

TEST(CandidateGeneration, TriangleNoDuplicates) {
    mags::Graph graph(3);
    graph[0] = {1, 2}; graph[1] = {0, 2}; graph[2] = {0, 1};

    const auto candidates = mags::cg::generate_candidates(graph, 10);

    // std::set deduplication ensures (u,v) is not added twice despite multiple paths
    EXPECT_EQ(candidates.size(), 3);
}

TEST(CandidateGeneration, IsolatedNode) {
    mags::Graph graph(5);
    graph[0] = {1}; graph[1] = {0};
    // Nodes 2, 3, 4 are isolated

    const auto candidates = mags::cg::generate_candidates(graph, 10);

    // Mags skips empty neighbors during sampling
    EXPECT_EQ(candidates.size(), 1); // Only (0, 1) should exist
}

TEST(CandidateGeneration, SeedingIsDeterministic) {
    mags::Graph graph(4);
    graph[0] = {1, 2}; graph[1] = {0, 3}; graph[2] = {0, 3}; graph[3] = {1, 2};

    // With a fixed seed (2333), the permutations must be identical every run
    const auto run1 = mags::cg::generate_candidates(graph, 10);
    const auto run2 = mags::cg::generate_candidates(graph, 10);

    EXPECT_EQ(run1.size(), run2.size());
    EXPECT_EQ(run1, run2);
}

TEST(CandidateGeneration, MinHashSignaturesWithinVertexRange) {
    mags::Graph graph(4);
    graph[0] = {1}; graph[1] = {0, 2}; graph[2] = {1, 3}; graph[3] = {2};

    mags::cg::SignatureMatrix signatures(graph.size(), std::vector<int>(mags::cg::H_FUNCS));
    mags::cg::compute_minhashes(graph, signatures);

    for (const auto& node_sig : signatures) {
        for (int rank : node_sig) {
            // Rank must be -1 (isolated) or within [0, n-1]
            EXPECT_TRUE(rank == -1 || (rank >= 0 && rank < 4));
        }
    }
}