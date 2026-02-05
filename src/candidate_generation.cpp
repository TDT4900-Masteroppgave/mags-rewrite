#include "mags/candidate_generation.h"
#include "mags/types.h"
#include "parallel_hashmap/btree.h"

#include <algorithm>
#include <numeric>
#include <random>
#include <unordered_set>
#include <vector>

namespace mags::cg {

namespace detail {
// Generates MinHash signatures to estimate Jaccard similarity between node
// neighborhoods
void compute_minhash(const Graph &graph, SignatureMatrix &signatures) {
  for (auto &row : signatures) {
    std::ranges::fill(row, -1);
  }

  std::vector<int> h_func(graph.size());
  std::iota(h_func.begin(), h_func.end(), 0);

  std::mt19937 rng(SEED);
  for (int h_idx = 0; h_idx < H_FUNCS; ++h_idx) {
    // Line 1: Initialize h hash functions
    std::ranges::shuffle(h_func, rng);

    // Line 2: Compute MinHashes
    for (int i = 0; i < h_func.size(); ++i) {
      for (const NodeID u = h_func.at(i); const NodeID nbr : graph.at(u)) {
        if (signatures.at(nbr).at(h_idx) == -1) {
          // uses minimum rank instead of minimum hash
          // nbr in rank-order 'i' ensures 'i' is the minimum rank for this
          // permutation by storing the minimum rank; one ensures that the
          // probability of two nodes sharing the same rank 'i' is equal to
          // their Jaccard similarity.
          signatures.at(nbr).at(h_idx) = i;
        }
      }
    }
  }
}

// MinHash-based estimate of Jaccard similarity between neighboring sets of
// nodes u and v
int mh_score(const NodeID u, const NodeID v,
             const SignatureMatrix &signatures) {
  int matches = 0;
  for (int h_idx = 0; h_idx < H_FUNCS; ++h_idx) {
    if (signatures.at(u).at(h_idx) == signatures.at(v).at(h_idx)) {
      matches++;
    }
  }
  return matches;
}

void get_two_hop_neighbors(const Graph &graph, const NodeID u, const int b,
                           std::unordered_set<NodeID> &two_hop_neighbors) {
  const std::vector<NodeID> &neighbors = graph.at(u);

  const int num_to_sample = std::min(b, static_cast<int>(neighbors.size()));

  // Line 6 (Part A): Let 2Hop <- N_u (include all immediate neighbors)
  for (NodeID one_hop : neighbors) {
    // Avoiding duplicate node pairs, as {u, v} = {v, u}
    if (one_hop > u)
      two_hop_neighbors.insert(one_hop);
  }

  for (int i = 0; i < num_to_sample; ++i) {
    // Line 5: Sample a random subset S of b nodes from N_u
    // Line 6 (Part B): 2Hop <- Union of N_w for w in S (include all neighbors
    // of the b sampled nodes in S)
    for (const NodeID one_hop = neighbors.at(i);
         const NodeID two_hop : graph.at(one_hop)) {
      // Avoiding duplicate node pairs, as {u, v} = {v, u}
      if (two_hop > u)
        two_hop_neighbors.insert(two_hop);
    }
  }
}

void get_top_k_candidate_pairs(
    const NodeID u, const int k, const SignatureMatrix &signatures,
    const std::unordered_set<NodeID> &two_hop_neighbors,
    phmap::btree_set<std::pair<int, NodeID>> &top_k) {
  // Line 7: For each neighboring node in 2Hop
  for (const NodeID v : two_hop_neighbors) {
    // Line 8: MinHash-based scoring (Equation 5)
    int score = mh_score(u, v, signatures);

    // Line 9: Select k nodes with highest mh(u,v)
    if (top_k.size() < k)
      top_k.emplace(score, v);
    else if (!top_k.empty() && score > top_k.begin()->first) {
      top_k.erase(top_k.begin());
      top_k.emplace(score, v);
    }
  }
}
} // namespace detail

CandidatePairSet generate_candidates(const Graph &graph, const int k) {
  const size_t n = graph.size();
  SignatureMatrix signatures(n, std::vector<int>(H_FUNCS));

  detail::compute_minhash(graph, signatures);

  // Line 3: Initialize a candidate pair set
  CandidatePairSet CP(n);

  // Line 4: For each node u in all nodes
  for (NodeID u = 0; u < static_cast<int>(n); ++u) {
    if (graph.at(u).empty())
      continue;

    std::unordered_set<NodeID> two_hop_neighbors;
    detail::get_two_hop_neighbors(graph, u, B_SAMPLE, two_hop_neighbors);

    phmap::btree_set<std::pair<int, NodeID>> top_k;
    detail::get_top_k_candidate_pairs(u, k, signatures, two_hop_neighbors,
                                      top_k);

    // Line 10: Put (u, v) into CP for each selected v
    for (const auto &[score, v] : top_k) {
      // TODO: insert saving
      CP.at(u).emplace(v, score);
      CP.at(v).emplace(u, score);
    }
  }
  return CP;
}
} // namespace mags::cg