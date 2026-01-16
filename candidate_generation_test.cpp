#include <gtest/gtest.h>
#include <algorithm>
#include "candidate_generation.h"

namespace {
    int count_signature_matches(const int u, const int v, const mags::cg::SignatureMatrix& sigs) {
        int matches = 0;
        for (int h = 0; h < mags::cg::H_FUNCS; ++h) {
            if (sigs.at(u).at(h) == sigs.at(v).at(h)) matches++;
        }
        return matches;
    }

    bool contains_pair(const mags::cg::CandidatePairSet& cp, int u, int v) {
        const std::pair<mags::NodeID, mags::NodeID> target = (u < v) ?
        std::make_pair(u, v) : std::make_pair(v, u);

        return cp.contains(target);
    }
}

class CandidateGenerationTest : public ::testing::Test {
protected:
    void SetUp() override {
        mags::cg::SEED = 2333;
    }
};


TEST_F(CandidateGenerationTest, IdenticalNeighbors) {
    // Graph with 6 nodes, node 0 and 1 has the same neighbors, and nodes 3 and 4 have the same neighbors
    const mags::Graph graph = {
        {2, 3},
        {2, 3},
        {0, 1},
        {0, 1}
    };

    const size_t n = graph.size();
    mags::cg::SignatureMatrix signatures(n, std::vector<int>(mags::cg::H_FUNCS));
    mags::cg::compute_minhashes(graph, signatures);

    for (int h = 0; h < mags::cg::H_FUNCS; ++h) {
        EXPECT_EQ(signatures.at(0).at(h), signatures.at(1).at(h));
        EXPECT_EQ(signatures.at(2).at(h), signatures.at(3).at(h));
    }
}

TEST_F(CandidateGenerationTest, DisjointNeighbors) {
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
    mags::cg::compute_minhashes(graph, signatures);

    EXPECT_EQ(count_signature_matches(0, 1, signatures), mags::cg::H_FUNCS);
    EXPECT_EQ(count_signature_matches(0, 4, signatures), 0);
}

TEST_F(CandidateGenerationTest, IsolatedNode) {
    // Graph with 6 nodes, node 2 is an isolated node and should remain unvisited (-1)
    const mags::Graph graph = {
        {0, 1},
        {0, 1},
        {}
    };
    const size_t n = graph.size();

    mags::cg::SignatureMatrix signatures(n, std::vector<int>(mags::cg::H_FUNCS));
    mags::cg::compute_minhashes(graph, signatures);

    for (int h = 0; h < mags::cg::H_FUNCS; ++h) {
        EXPECT_EQ(signatures.at(2).at(h), -1);
    }
}

TEST_F(CandidateGenerationTest,SmallestGraph) {
    const mags::Graph graph = {
        {0}
    };
    const size_t n = graph.size();

    mags::cg::SignatureMatrix signatures(n, std::vector<int>(mags::cg::H_FUNCS));
    mags::cg::compute_minhashes(graph, signatures);

    for (int h = 0; h < mags::cg::H_FUNCS; ++h) {
        EXPECT_EQ(signatures.at(0).at(h), 0);
    }
}

TEST_F(CandidateGenerationTest, SeedConsistency) {
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

TEST_F(CandidateGenerationTest, SelfLoopChangesSimilarity) {
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

    mags::cg::compute_minhashes(g_no_self_loop, sig1);
    mags::cg::compute_minhashes(g_with_self_loop, sig2);

    EXPECT_NE(sig1.at(1), sig2.at(1));
    EXPECT_NE(count_signature_matches(0, 1, sig1), count_signature_matches(0, 1, sig2));
}

TEST_F(CandidateGenerationTest, DiamonGraphCompleteness) {
    const mags::Graph graph = {
        {1, 2},
        {0, 3},
        {0, 3},
        {1, 2}
    };
    const size_t n = graph.size();

    mags::cg::SignatureMatrix signatures(n, std::vector<int>(mags::cg::H_FUNCS));
    mags::cg::compute_minhashes(graph, signatures);

    EXPECT_EQ(count_signature_matches(0, 3, signatures), mags::cg::H_FUNCS);
}

TEST_F(CandidateGenerationTest, IdenticalSignature) {
    constexpr size_t n = 2;
    const mags::cg::SignatureMatrix sigs(n, std::vector(mags::cg::H_FUNCS, 1));

    const int score = mags::cg::mh_score(0, 1, sigs);
    EXPECT_EQ(score, mags::cg::H_FUNCS);
}

TEST_F(CandidateGenerationTest, DisjointSignature) {
    constexpr size_t n = 2;
    mags::cg::SignatureMatrix sigs(n, std::vector<int>(mags::cg::H_FUNCS));

    for (int h = 0; h < mags::cg::H_FUNCS; ++h) {
        sigs.at(0).at(h) = h;
        sigs.at(1).at(h) = h + 1;
    }

    const int score = mags::cg::mh_score(0, 1, sigs);
    EXPECT_EQ(score, 0);
}

TEST_F(CandidateGenerationTest, PartialOverlap) {
    constexpr size_t n = 2;
    mags::cg::SignatureMatrix sigs(n, std::vector<int>(mags::cg::H_FUNCS));

    for (int h = 0; h < mags::cg::H_FUNCS; ++h) {
        sigs.at(0).at(h) = h;
        sigs.at(1).at(h) = (h < 20) ? h: 0;
    }

    const int score = mags::cg::mh_score(0, 1, sigs);
    EXPECT_EQ(score, 20);
}

TEST_F(CandidateGenerationTest, Symmetry) {
    constexpr size_t n = 2;
    mags::cg::SignatureMatrix sigs(n, std::vector<int>(mags::cg::H_FUNCS));

    std::mt19937 rng(mags::cg::SEED);

    for (int h = 0; h < mags::cg::H_FUNCS; ++h) {
        sigs.at(0).at(h) = static_cast<int>(rng() % 20);
        sigs.at(1).at(h) = static_cast<int>(rng() % 15);
    }

    EXPECT_EQ(mags::cg::mh_score(0, 1, sigs), mags::cg::mh_score(1, 0, sigs));
}

TEST_F(CandidateGenerationTest, IsolatedNodesMatch) {
    constexpr size_t n = 2;
    const mags::cg::SignatureMatrix sigs(n, std::vector(mags::cg::H_FUNCS, -1));

    const int score = mags::cg::mh_score(0, 1, sigs);
    EXPECT_EQ(score, mags::cg::H_FUNCS);
}

TEST_F(CandidateGenerationTest, SelfSimilarity) {
    constexpr size_t n = 1;
    mags::cg::SignatureMatrix sigs(n, std::vector<int>(mags::cg::H_FUNCS));

    std::mt19937 rng(mags::cg::SEED);
    for (int h = 0; h < mags::cg::H_FUNCS; ++h) {
        sigs.at(0).at(h) = static_cast<int>(rng() % 20);
    }

    const int score = mags::cg::mh_score(0, 0, sigs);
    EXPECT_EQ(score, mags::cg::H_FUNCS);
}

TEST_F(CandidateGenerationTest,VisitedVsUnvisited) {
    constexpr size_t n = 2;
    mags::cg::SignatureMatrix sigs(n, std::vector<int>(mags::cg::H_FUNCS));

    for (int h = 0; h < mags::cg::H_FUNCS; ++h) {
        sigs.at(0).at(h) = h;
        sigs.at(1).at(h) = -1;
    }

    const int score = mags::cg::mh_score(0, 1, sigs);
    EXPECT_EQ(score, 0);
}

TEST_F(CandidateGenerationTest, DiamondGraphCompleteness) {
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

TEST_F(CandidateGenerationTest,NoInvalidPairs) {
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

TEST_F(CandidateGenerationTest, TriangleNoDuplicates) {
    mags::Graph graph(3);
    graph[0] = {1, 2}; graph[1] = {0, 2}; graph[2] = {0, 1};

    const auto candidates = mags::cg::generate_candidates(graph, 10);

    // std::set deduplication ensures (u,v) is not added twice despite multiple paths
    EXPECT_EQ(candidates.size(), 3);
}

TEST_F(CandidateGenerationTest, IsolatedNodeSize) {
    mags::Graph graph(5);
    graph[0] = {1}; graph[1] = {0};
    // Nodes 2, 3, 4 are isolated

    const auto candidates = mags::cg::generate_candidates(graph, 10);

    // Mags skips empty neighbors during sampling
    EXPECT_EQ(candidates.size(), 1); // Only (0, 1) should exist
}

TEST_F(CandidateGenerationTest, SeedingIsDeterministic) {
    mags::Graph graph(4);
    graph[0] = {1, 2}; graph[1] = {0, 3}; graph[2] = {0, 3}; graph[3] = {1, 2};

    const auto run1 = mags::cg::generate_candidates(graph, 10);
    const auto run2 = mags::cg::generate_candidates(graph, 10);

    EXPECT_EQ(run1.size(), run2.size());
    EXPECT_EQ(run1, run2);
}

TEST_F(CandidateGenerationTest, MinHashSignaturesWithinVertexRange) {
    mags::Graph graph(4);
    graph[0] = {1}; graph[1] = {0, 2}; graph[2] = {1, 3}; graph[3] = {2};

    mags::cg::SignatureMatrix signatures(graph.size(), std::vector<int>(mags::cg::H_FUNCS));
    mags::cg::compute_minhashes(graph, signatures);

    for (const auto& node_sig : signatures) {
        for (const int rank : node_sig) {
            // Rank must be -1 (isolated) or within [0, n-1]
            EXPECT_TRUE(rank == -1 || (rank >= 0 && rank < 4));
        }
    }
}

TEST_F(CandidateGenerationTest, KLimitPerNode) {
    mags::Graph graph(100);

    // Star graph, center node 0 has 99 neighbors
    for (int i = 0; i < graph.size(); ++i) {
        graph.at(0).push_back(i);
        graph.at(i).push_back(0);
    }

    constexpr int k = 10;
    const auto candidates = mags::cg::generate_candidates(graph, k);

    // Should max generate n*k candidate pairs
    EXPECT_LE(candidates.size(), graph.size() * k);

    // Count how many pairs involve node 1
    int node_1_candidates = 0;
    for (const auto& [u, v] : candidates) {
        if (u == 1 || v == 1) node_1_candidates++;
    }

    EXPECT_LE(node_1_candidates, k);
}

TEST_F(CandidateGenerationTest, VerifyUndirectionalFilter) {
    mags::Graph graph = {
        {1, 2},
        {0, 2},
        {0, 1}
    };

    for (const auto candidates = mags::cg::generate_candidates(graph, 10);
        const auto& [u, v] : candidates) {
        EXPECT_TRUE(u < v);
    }
}

TEST_F(CandidateGenerationTest, NoSelfPairs) {
    mags::Graph graph = {
        {0, 1},
        {0, 1}
    };

    for (const auto candidates = mags::cg::generate_candidates(graph, 10);
        const auto& [u, v] : candidates) {
        EXPECT_NE(u, v);
    }
}

TEST_F(CandidateGenerationTest, TopKEviction) {
    mags::Graph graph(20);
    graph.at(0) = {10, 11, 12, 13, 14};
    for (const int nbr : graph.at(0)) graph.at(nbr).push_back(0);

    // Candidate 1: shares 1 neighbor with node 0 (Low similarity)
    graph[1] = {10}; graph[10].push_back(1);

    // Candidate 2: shares 2 neighbors with node 0
    graph[2] = {10, 11}; graph[10].push_back(2); graph[11].push_back(2);

    // Candidate 3: shares 3 neighbors with node 0
    graph[3] = {10, 11, 12}; graph[10].push_back(3); graph[11].push_back(3); graph[12].push_back(3);

    // Candidate 4: shares 4 neighbors with node 0
    graph[4] = {10, 11, 12, 13};
    graph[10].push_back(4); graph[11].push_back(4);
    graph[12].push_back(4); graph[13].push_back(4);

    // Candidate 5: shares all 5 neighbors (Highest similarity)
    graph[5] = {10, 11, 12, 13, 14};
    for (const int nbr : graph[0]) graph[nbr].push_back(5);

    constexpr int k = 1;
    const auto candidates = mags::cg::generate_candidates(graph, k);

    EXPECT_FALSE(candidates.empty());

    bool found_best = false;
    for (const auto& [u, v] : candidates) {
        if ((u == 0 && v == 5) || (u == 5 && v == 0)) found_best = true;
    }
    EXPECT_TRUE(found_best);
}