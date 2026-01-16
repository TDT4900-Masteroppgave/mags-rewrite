#include <gtest/gtest.h>
#include <algorithm>
#include "mags/candidate_generation.h"

#include "mags/util.h"

namespace {
    class CandidateGenerationTest : public ::testing::Test {
    protected:
        mags::Graph diamond_graph;
        mags::Graph triangle_graph;
        mags::Graph star_graph;
        mags::Graph path_graph;
        mags::Graph ladder_graph;
        mags::Graph isolated_graph;

        void SetUp() override {
            mags::cg::SEED = 2333;

            diamond_graph = {
                {1, 2}, {0, 3}, {0, 3}, {1, 2}
            };

            triangle_graph = {
                {1, 2}, {0, 2}, {0, 1}
            };

            star_graph.assign(11, {});
            for (int i = 1; i <= 10; ++i) {
                star_graph.at(0).push_back(i);
                star_graph.at(i).push_back(0);
            }

            path_graph = {
                {1}, {0, 2}, {1, 3}, {2}
            };

            ladder_graph = create_ladder_graph();

            isolated_graph = {
                {1},
                {0},
                {},
                {},
                {}

            };

            prepare_graph(diamond_graph);
            prepare_graph(triangle_graph);
            prepare_graph(star_graph);
            prepare_graph(path_graph);
            prepare_graph(ladder_graph);
            prepare_graph(isolated_graph);
        }

        static mags::Graph create_ladder_graph() {
            mags::Graph g;
            g.assign(15, {});

            g.at(0) = {10, 11, 12, 14, 14};
            for (const int nbr : g.at(0)) g.at(nbr).push_back(0);

            for (int i = 1; i <= 5; ++i) {
                for (int j = 0; j < i; ++j) {
                    constexpr int shared_nbr = 10;
                    g.at(i).push_back(shared_nbr);
                    g.at(shared_nbr).push_back(i);
                }
            }
            return g;
        }

        static void prepare_graph(mags::Graph& g) {
            mags::util::sort_neighbors(g);
        }
    };

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




TEST_F(CandidateGenerationTest, IdenticalNeighbors) {
    // g with 6 nodes, node 0 and 1 has the same neighbors, and nodes 3 and 4 have the same neighbors
    mags::Graph g = {
        {2, 3},
        {2, 3},
        {0, 1},
        {0, 1}
    };

    prepare_graph(g);

    const size_t n = g.size();
    mags::cg::SignatureMatrix sigs(n, std::vector<int>(mags::cg::H_FUNCS));
    mags::cg::detail::compute_minhashes(g, sigs);

    for (int h = 0; h < mags::cg::H_FUNCS; ++h) {
        EXPECT_EQ(sigs.at(0).at(h), sigs.at(1).at(h));
        EXPECT_EQ(sigs.at(2).at(h), sigs.at(3).at(h));
    }
}

TEST_F(CandidateGenerationTest, DisjointNeighbors) {
    // g with 6 nodes, node 0 and 1 has the same neighbors, and nodes 3 and 4 have the same neighbors
    const mags::Graph g = {
        {2, 3},
        {2, 3},
        {0, 1},
        {0, 1},
        {4}
    };
    const size_t n = g.size();

    mags::cg::SignatureMatrix sigs(n, std::vector<int>(mags::cg::H_FUNCS));
    mags::cg::detail::compute_minhashes(g, sigs);

    EXPECT_EQ(count_signature_matches(1, 0, sigs), mags::cg::H_FUNCS);
    EXPECT_EQ(count_signature_matches(0, 4, sigs), 0);
}

TEST_F(CandidateGenerationTest, IsolatedNode) {
    // Node 2, 3, 4 is an isolated node and should remain unvisited (-1)
    const auto g = isolated_graph;
    const size_t n = g.size();

    mags::cg::SignatureMatrix sigs(n, std::vector<int>(mags::cg::H_FUNCS));
    mags::cg::detail::compute_minhashes(g, sigs);

    for (int h = 0; h < mags::cg::H_FUNCS; ++h) {
        EXPECT_EQ(sigs.at(2).at(h), -1);
        EXPECT_EQ(sigs.at(3).at(h), -1);
        EXPECT_EQ(sigs.at(4).at(h), -1);
    }
}

TEST_F(CandidateGenerationTest, SmallestGraph) {
    const mags::Graph g = {
        {0}
    };
    const size_t n = g.size();

    mags::cg::SignatureMatrix sigs(n, std::vector<int>(mags::cg::H_FUNCS));
    mags::cg::detail::compute_minhashes(g, sigs);

    for (int h = 0; h < mags::cg::H_FUNCS; ++h) {
        EXPECT_EQ(sigs.at(0).at(h), 0);
    }
}

TEST_F(CandidateGenerationTest, SeedConsistency) {
    const mags::Graph g = {
        {1, 2},
        {0, 1},
        {1}
    };
    const size_t n = g.size();

    mags::cg::SignatureMatrix sig_seed_0_run_a(n, std::vector<int>(mags::cg::H_FUNCS));
    mags::cg::SignatureMatrix sig_seed_0_run_b(n, std::vector<int>(mags::cg::H_FUNCS));
    mags::cg::SignatureMatrix sig_seed_1(n, std::vector<int>(mags::cg::H_FUNCS));

    mags::cg::SEED = 0;
    mags::cg::detail::compute_minhashes(g, sig_seed_0_run_a);
    mags::cg::detail::compute_minhashes(g, sig_seed_0_run_b);

    mags::cg::SEED = 1;
    mags::cg::detail::compute_minhashes(g, sig_seed_1);

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

    mags::cg::detail::compute_minhashes(g_no_self_loop, sig1);
    mags::cg::detail::compute_minhashes(g_with_self_loop, sig2);

    EXPECT_NE(sig1.at(1), sig2.at(1));
    EXPECT_NE(count_signature_matches(0, 1, sig1), count_signature_matches(0, 1, sig2));
}

TEST_F(CandidateGenerationTest, DiamonGraphCompleteness) {
    const auto g = diamond_graph;
    mags::cg::SignatureMatrix sigs(g.size(), std::vector<int>(mags::cg::H_FUNCS));
    mags::cg::detail::compute_minhashes(g, sigs);

    EXPECT_EQ(count_signature_matches(0, 3, sigs), mags::cg::H_FUNCS);
}

TEST_F(CandidateGenerationTest, IdenticalSignature) {
    constexpr size_t n = 2;
    const mags::cg::SignatureMatrix sigs(n, std::vector(mags::cg::H_FUNCS, 1));

    const int score = mags::cg::detail::mh_score(0, 1, sigs);
    EXPECT_EQ(score, mags::cg::H_FUNCS);
}

TEST_F(CandidateGenerationTest, DisjointSignature) {
    constexpr size_t n = 2;
    mags::cg::SignatureMatrix sigs(n, std::vector<int>(mags::cg::H_FUNCS));

    for (int h = 0; h < mags::cg::H_FUNCS; ++h) {
        sigs.at(0).at(h) = h;
        sigs.at(1).at(h) = h + 1;
    }

    const int score = mags::cg::detail::mh_score(0, 1, sigs);
    EXPECT_EQ(score, 0);
}

TEST_F(CandidateGenerationTest, PartialOverlap) {
    constexpr size_t n = 2;
    mags::cg::SignatureMatrix sigs(n, std::vector<int>(mags::cg::H_FUNCS));

    for (int h = 0; h < mags::cg::H_FUNCS; ++h) {
        sigs.at(0).at(h) = h;
        sigs.at(1).at(h) = (h < 20) ? h: 0;
    }

    const int score = mags::cg::detail::mh_score(0, 1, sigs);
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

    EXPECT_EQ(mags::cg::detail::mh_score(0, 1, sigs), mags::cg::detail::mh_score(1, 0, sigs));
}

TEST_F(CandidateGenerationTest, IsolatedNodesMatch) {
    constexpr size_t n = 2;
    const mags::cg::SignatureMatrix sigs(n, std::vector(mags::cg::H_FUNCS, -1));

    const int score = mags::cg::detail::mh_score(0, 1, sigs);
    EXPECT_EQ(score, mags::cg::H_FUNCS);
}

TEST_F(CandidateGenerationTest, SelfSimilarity) {
    constexpr size_t n = 1;
    mags::cg::SignatureMatrix sigs(n, std::vector<int>(mags::cg::H_FUNCS));

    std::mt19937 rng(mags::cg::SEED);
    for (int h = 0; h < mags::cg::H_FUNCS; ++h) {
        sigs.at(0).at(h) = static_cast<int>(rng() % 20);
    }

    const int score = mags::cg::detail::mh_score(0, 0, sigs);
    EXPECT_EQ(score, mags::cg::H_FUNCS);
}

TEST_F(CandidateGenerationTest,VisitedVsUnvisited) {
    constexpr size_t n = 2;
    mags::cg::SignatureMatrix sigs(n, std::vector<int>(mags::cg::H_FUNCS));

    for (int h = 0; h < mags::cg::H_FUNCS; ++h) {
        sigs.at(0).at(h) = h;
        sigs.at(1).at(h) = -1;
    }

    const int score = mags::cg::detail::mh_score(0, 1, sigs);
    EXPECT_EQ(score, 0);
}

TEST_F(CandidateGenerationTest, TwoHopSamplingLimit) {
    const auto g = star_graph; // Node 0 is connected to 10 nodes
    std::unordered_set<mags::NodeID> neighbors;

    // Sample only 2 neighbors of Node 0
    mags::cg::detail::get_two_hop_neighbors(g, 0, 2, neighbors);

    // Should contain all 10 immediate neighbors (1-hop is always included),
    // but the 2-hop logic should have only triggered for 2 of those neighbors.
    EXPECT_GE(neighbors.size(), 10);
}

TEST_F(CandidateGenerationTest, TopKPriorityEviction) {
    constexpr mags::NodeID u = 0;
    constexpr int k = 2;
    std::unordered_set<mags::NodeID> neighbors = {1, 2, 3};

    // Create a dummy signature matrix where 1 and 2 are better than 3
    mags::cg::SignatureMatrix sigs(4, std::vector<int>(mags::cg::H_FUNCS, 0));
    // Node 1: Perfect match
    std::ranges::fill(sigs.at(1), 0);
    // Node 2: Half match
    for(int i=0; i < mags::cg::H_FUNCS/2; ++i) sigs.at(2).at(i) = 1;
    // Node 3: No match
    std::ranges::fill(sigs.at(3), 2);

    phmap::btree_set<std::pair<int, mags::NodeID>> top_k;
    mags::cg::detail::get_top_k_candidate_pairs(u, k, neighbors, sigs, top_k);

    EXPECT_EQ(top_k.size(), k);
    // Node 3 should have been evicted because its score is 0
    for (const auto& [score, v] : top_k) {
        EXPECT_NE(v, 3);
    }
}

TEST_F(CandidateGenerationTest, DiamondGraphCompleteness) {
    const auto g = diamond_graph;

    const auto candidates = mags::cg::generate_candidates(g, 30);

    EXPECT_EQ(candidates.size(), g.size()*(g.size()-1)/2);
    EXPECT_TRUE(contains_pair(candidates, 3, 0));
    EXPECT_TRUE(contains_pair(candidates, 1, 2));
}

TEST_F(CandidateGenerationTest, NoInvalidPairs) {
    const auto g = path_graph;

    for (const auto candidates = mags::cg::generate_candidates(g, 10);
        const auto& [u, v] : candidates) {
        // Enforce u < v normalization
        EXPECT_TRUE(u < v);
        EXPECT_GE(u, 0);
        EXPECT_LT(v, g.size());
    }
}

TEST_F(CandidateGenerationTest, TriangleNoDuplicates) {
    const auto g = triangle_graph;

    const auto candidates = mags::cg::generate_candidates(g, 10);

    // std::set deduplication ensures (u,v) is not added twice despite multiple paths
    EXPECT_EQ(candidates.size(), g.size() * (g.size() - 1)  / 2);
}

TEST_F(CandidateGenerationTest, IsolatedNodeSize) {
    const auto g = isolated_graph;

    const auto candidates = mags::cg::generate_candidates(g, 10);

    // Mags skips empty neighbors during sampling
    EXPECT_EQ(candidates.size(), 1); // Only (0, 1) should exist
}

TEST_F(CandidateGenerationTest, SeedingIsDeterministic) {
    const auto g = path_graph;

    const auto run1 = mags::cg::generate_candidates(g, 10);
    const auto run2 = mags::cg::generate_candidates(g, 10);

    EXPECT_EQ(run1.size(), run2.size());
    EXPECT_EQ(run1, run2);
}

TEST_F(CandidateGenerationTest, MinHashSignaturesWithinVertexRange) {
    const auto g = path_graph;

    mags::cg::SignatureMatrix sigs(g.size(), std::vector<int>(mags::cg::H_FUNCS));
    mags::cg::detail::compute_minhashes(g, sigs);

    for (const auto& node_sig : sigs) {
        for (const int rank : node_sig) {
            // Rank is either -1 (no neighbor was the 'min' for this hash)
            // or a valid index in the graph vector [0, n-1]
            EXPECT_TRUE(rank >= -1 && rank < g.size());
        }
    }
}

TEST_F(CandidateGenerationTest, KLimitPerNode) {
    const auto g = star_graph;

    constexpr int k = 10;
    const auto candidates = mags::cg::generate_candidates(g, k);

    // Generate max n * k candidate pairs
    EXPECT_LE(candidates.size(), g.size() * k);

    // Count how many pairs involve node 1
    int node_1_candidates = 0;
    for (const auto& [u, v] : candidates) {
        if (u == 1 || v == 1) node_1_candidates++;
    }

    EXPECT_LE(node_1_candidates, k);
}

TEST_F(CandidateGenerationTest, VerifyUndirectionalFilter) {
    const auto g = triangle_graph;

    for (const auto candidates = mags::cg::generate_candidates(g, 10);
        const auto& [u, v] : candidates) {
        EXPECT_TRUE(u < v);
    }
}

TEST_F(CandidateGenerationTest, NoSelfPairs) {
    const mags::Graph g = {
        {0, 1},
        {0, 1}
    };

    for (const auto candidates = mags::cg::generate_candidates(g, 10);
        const auto& [u, v] : candidates) {
        EXPECT_NE(u, v);
    }
}

TEST_F(CandidateGenerationTest, TopKEviction) {
    const auto g = ladder_graph;

    constexpr int k = 1;
    const auto candidates = mags::cg::generate_candidates(g, k);

    EXPECT_FALSE(candidates.empty());

    bool found_best = false;
    for (const auto& [u, v] : candidates) {
        if ((u == 0 && v == 5) || (u == 5 && v == 0)) found_best = true;
    }
    EXPECT_TRUE(found_best);
}