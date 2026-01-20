#include "GraphTestUtility.h"

#include "mags/candidate_generation.h"
#include <algorithm>
#include <gtest/gtest.h>

namespace mags::test {
using namespace mags::cg;
using namespace detail;

class CandidateGenerationTest : public GraphTestUtility {};

TEST_F(CandidateGenerationTest, IdenticalNeighbors) {
  // g with 6 nodes, node 0 and 1 has the same neighbors, and nodes 3 and 4 have
  // the same neighbors
  const Graph g = {{2, 3}, {2, 3}, {0, 1}, {0, 1}};

  const size_t n = g.size();
  SignatureMatrix sigs(n, std::vector<int>(H_FUNCS));
  compute_minhashes(g, sigs);

  for (int h = 0; h < H_FUNCS; ++h) {
    EXPECT_EQ(sigs.at(0).at(h), sigs.at(1).at(h));
    EXPECT_EQ(sigs.at(2).at(h), sigs.at(3).at(h));
  }
}

TEST_F(CandidateGenerationTest, DisjointNeighbors) {
  // g with 6 nodes, node 0 and 1 has the same neighbors, and nodes 3 and 4 have
  // the same neighbors
  const Graph g = {{2, 3}, {2, 3}, {0, 1}, {0, 1}, {4}};
  const size_t n = g.size();

  SignatureMatrix sigs(n, std::vector<int>(H_FUNCS));
  compute_minhashes(g, sigs);

  EXPECT_EQ(signature_matches(1, 0, sigs), H_FUNCS);
  EXPECT_EQ(signature_matches(0, 4, sigs), 0);
}

TEST_F(CandidateGenerationTest, IsolatedNode) {
  // Node 2, 3, 4 is an isolated node and should remain unvisited (-1)
  const auto g = isolated;
  const size_t n = g.size();

  SignatureMatrix sigs(n, std::vector<int>(H_FUNCS));
  compute_minhashes(g, sigs);

  for (int h = 0; h < H_FUNCS; ++h) {
    EXPECT_EQ(sigs.at(2).at(h), -1);
    EXPECT_EQ(sigs.at(3).at(h), -1);
    EXPECT_EQ(sigs.at(4).at(h), -1);
  }
}

TEST_F(CandidateGenerationTest, SmallestGraph) {
  const Graph g = {{0}};

  const auto candidates = generate_candidates(g, 10);
  EXPECT_EQ(candidates.size(), 0);
}

TEST_F(CandidateGenerationTest, SeedConsistency) {
  const Graph g = {{1, 2}, {0, 1}, {1}};
  const size_t n = g.size();

  SignatureMatrix sig_seed_0_run_a(n, std::vector<int>(H_FUNCS));
  SignatureMatrix sig_seed_0_run_b(n, std::vector<int>(H_FUNCS));
  SignatureMatrix sig_seed_1(n, std::vector<int>(H_FUNCS));

  SEED = 0;
  compute_minhashes(g, sig_seed_0_run_a);
  compute_minhashes(g, sig_seed_0_run_b);

  SEED = 1;
  compute_minhashes(g, sig_seed_1);

  EXPECT_EQ(sig_seed_0_run_a, sig_seed_0_run_b);
  EXPECT_NE(sig_seed_0_run_a, sig_seed_1);
}

TEST_F(CandidateGenerationTest, SelfLoopChangesSimilarity) {
  const Graph g_no_self_loop = {{2}, {2}, {0, 1}};
  const size_t n1 = g_no_self_loop.size();

  const Graph g_with_self_loop = {{2, 0}, {2, 1}, {0, 1}};
  const size_t n2 = g_with_self_loop.size();

  SignatureMatrix sig1(n1, std::vector<int>(H_FUNCS));
  SignatureMatrix sig2(n2, std::vector<int>(H_FUNCS));

  compute_minhashes(g_no_self_loop, sig1);
  compute_minhashes(g_with_self_loop, sig2);

  EXPECT_NE(sig1.at(1), sig2.at(1));
  EXPECT_NE(signature_matches(0, 1, sig1), signature_matches(0, 1, sig2));
}

TEST_F(CandidateGenerationTest, DiamonGraphCompleteness) {
  const auto g = diamond;
  SignatureMatrix sigs(g.size(), std::vector<int>(H_FUNCS));
  compute_minhashes(g, sigs);

  EXPECT_EQ(signature_matches(0, 3, sigs), H_FUNCS);
}

TEST_F(CandidateGenerationTest, IdenticalSignature) {
  constexpr size_t n = 2;
  const SignatureMatrix sigs(n, std::vector(H_FUNCS, 1));

  const int score = mh_score(0, 1, sigs);
  EXPECT_EQ(score, H_FUNCS);
}

TEST_F(CandidateGenerationTest, DisjointSignature) {
  constexpr size_t n = 2;
  SignatureMatrix sigs(n, std::vector<int>(H_FUNCS));

  for (int h = 0; h < H_FUNCS; ++h) {
    sigs.at(0).at(h) = h;
    sigs.at(1).at(h) = h + 1;
  }

  const int score = mh_score(0, 1, sigs);
  EXPECT_EQ(score, 0);
}

TEST_F(CandidateGenerationTest, PartialOverlap) {
  constexpr size_t n = 2;
  SignatureMatrix sigs(n, std::vector<int>(H_FUNCS));

  for (int h = 0; h < H_FUNCS; ++h) {
    sigs.at(0).at(h) = h;
    sigs.at(1).at(h) = h < 20 ? h : 0;
  }

  const int score = mh_score(0, 1, sigs);
  EXPECT_EQ(score, 20);
}

TEST_F(CandidateGenerationTest, Symmetry) {
  constexpr size_t n = 2;
  SignatureMatrix sigs(n, std::vector<int>(H_FUNCS));

  std::mt19937 rng(SEED);

  for (int h = 0; h < H_FUNCS; ++h) {
    sigs.at(0).at(h) = static_cast<int>(rng() % 20);
    sigs.at(1).at(h) = static_cast<int>(rng() % 15);
  }

  EXPECT_EQ(mh_score(0, 1, sigs),
            mh_score(1, 0, sigs));
}

TEST_F(CandidateGenerationTest, IsolatedNodesMatch) {
  constexpr size_t n = 2;
  const SignatureMatrix sigs(n, std::vector(H_FUNCS, -1));

  const int score = mh_score(0, 1, sigs);
  EXPECT_EQ(score, H_FUNCS);
}

TEST_F(CandidateGenerationTest, SelfSimilarity) {
  constexpr size_t n = 1;
  SignatureMatrix sigs(n, std::vector<int>(H_FUNCS));

  std::mt19937 rng(SEED);
  for (int h = 0; h < H_FUNCS; ++h) {
    sigs.at(0).at(h) = static_cast<int>(rng() % 20);
  }

  const int score = mh_score(0, 0, sigs);
  EXPECT_EQ(score, H_FUNCS);
}

TEST_F(CandidateGenerationTest, VisitedVsUnvisited) {
  constexpr size_t n = 2;
  SignatureMatrix sigs(n, std::vector<int>(H_FUNCS));

  for (int h = 0; h < H_FUNCS; ++h) {
    sigs.at(0).at(h) = h;
    sigs.at(1).at(h) = -1;
  }

  const int score = mh_score(0, 1, sigs);
  EXPECT_EQ(score, 0);
}

TEST_F(CandidateGenerationTest, TwoHopSamplingLimit) {
  const auto g = star; // Node 0 is connected to 10 nodes
  std::unordered_set<NodeID> neighbors;

  // Sample only 2 neighbors of Node 0
  get_two_hop_neighbors(g, 0, 2, neighbors);

  // Should contain all 10 immediate neighbors (1-hop is always included),
  // but the 2-hop logic should have only triggered for 2 of those neighbors.
  EXPECT_GE(neighbors.size(), 10);
}

TEST_F(CandidateGenerationTest, TopKPriorityEviction) {
  constexpr NodeID u = 0;
  constexpr int k = 2;
  const std::unordered_set neighbors = {1, 2, 3};

  // Create a dummy signature matrix where 1 and 2 are better than 3
  SignatureMatrix sigs(4, std::vector(H_FUNCS, 0));
  // Node 1: Perfect match
  std::ranges::fill(sigs.at(1), 0);
  // Node 2: Half match
  for (int i = 0; i < H_FUNCS / 2; ++i)
    sigs.at(2).at(i) = 1;
  // Node 3: No match
  std::ranges::fill(sigs.at(3), 2);

  phmap::btree_set<std::pair<int, NodeID>> top_k;
  get_top_k_candidate_pairs(u, k, sigs, neighbors, top_k);

  EXPECT_EQ(top_k.size(), k);
  // Node 3 should have been evicted because its score is 0
  for (const auto &[score, v] : top_k) {
    EXPECT_NE(v, 3);
  }
}

TEST_F(CandidateGenerationTest, DiamondGraphCompleteness) {
  const auto g = diamond;

  const auto candidates = generate_candidates(g, 30);

  EXPECT_EQ(candidates.size(), g.size() * (g.size() - 1) / 2);
  EXPECT_TRUE(has_pair(candidates, 3, 0));
  EXPECT_TRUE(has_pair(candidates, 1, 2));
}

TEST_F(CandidateGenerationTest, NoInvalidPairs) {
  const auto g = path;

  for (const auto candidates = generate_candidates(g, 10);
       const auto &[u, v] : candidates) {
    // Enforce u < v normalization
    EXPECT_TRUE(u < v);
    EXPECT_GE(u, 0);
    EXPECT_LT(v, g.size());
  }
}

TEST_F(CandidateGenerationTest, TriangleNoDuplicates) {
  const auto g = triangle;

  const auto candidates = generate_candidates(g, 10);

  // std::set deduplication ensures (u,v) is not added twice despite multiple
  // paths
  EXPECT_EQ(candidates.size(), g.size() * (g.size() - 1) / 2);
}

TEST_F(CandidateGenerationTest, IsolatedNodeSize) {
  const auto g = isolated;

  const auto candidates = generate_candidates(g, 10);

  // Mags skips empty neighbors during sampling
  EXPECT_EQ(candidates.size(), 1); // Only (0, 1) should exist
}

TEST_F(CandidateGenerationTest, SeedingIsDeterministic) {
  const auto g = path;

  const auto run1 = generate_candidates(g, 10);
  const auto run2 = generate_candidates(g, 10);

  EXPECT_EQ(run1.size(), run2.size());
  EXPECT_EQ(run1, run2);
}

TEST_F(CandidateGenerationTest, MinHashSignaturesWithinVertexRange) {
  const auto g = path;

  SignatureMatrix sigs(g.size(), std::vector<int>(H_FUNCS));
  compute_minhashes(g, sigs);

  for (const auto &node_sig : sigs) {
    for (const int rank : node_sig) {
      // Rank is either -1 (no neighbor was the 'min' for this hash)
      // or a valid index in the graph vector [0, n-1]
      EXPECT_TRUE(rank >= -1 && rank < g.size());
    }
  }
}

TEST_F(CandidateGenerationTest, KLimitPerNode) {
  const auto g = star;

  constexpr int k = 10;
  const auto candidates = generate_candidates(g, k);

  // Generate max n * k candidate pairs
  EXPECT_LE(candidates.size(), g.size() * k);

  // Count how many pairs involve node 1
  int node_1_candidates = 0;
  for (const auto &[u, v] : candidates) {
    if (u == 1 || v == 1)
      node_1_candidates++;
  }

  EXPECT_LE(node_1_candidates, k);
}

TEST_F(CandidateGenerationTest, VerifyUndirectionalFilter) {
  const auto g = triangle;

  for (const auto candidates = generate_candidates(g, 10);
       const auto &[u, v] : candidates) {
    EXPECT_TRUE(u < v);
  }
}

TEST_F(CandidateGenerationTest, NoSelfPairs) {
  const Graph g = {{0, 1}, {0, 1}};

  for (const auto candidates = generate_candidates(g, 10);
       const auto &[u, v] : candidates) {
    EXPECT_NE(u, v);
  }
}

TEST_F(CandidateGenerationTest, TopKEviction) {
  const auto g = ladder;

  constexpr int k = 1;
  const auto candidates = generate_candidates(g, k);

  EXPECT_FALSE(candidates.empty());

  bool found_best = false;
  for (const auto &[u, v] : candidates) {
    if ((u == 0 && v == 5) || (u == 5 && v == 0))
      found_best = true;
  }
  EXPECT_TRUE(found_best);
}
} // namespace mags::test