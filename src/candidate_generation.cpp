#include "mags/candidate_generation.h"
#include "parallel_hashmap/btree.h"

#include <vector>
#include <random>
#include <queue>
#include <unordered_set>
#include <algorithm>

// TODO: duplicate - understand if (u != v)?, signatures.at(nbr).at(h_idx) = i; - understand
// TODO: refactoring - function for 2Hop generation, and function for TopK selection
namespace mags::cg {

    // Generates MinHash signatures to estimate Jaccard similarity between node neighborhoods
    void compute_minhashes(const Graph& graph, SignatureMatrix& signatures) {
        for (auto &row: signatures) {
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
                        signatures.at(nbr).at(h_idx) = i;
                    }
                }
            }
        }
    }

    // MinHash-based estimate of Jaccard similarity between neighboring sets of nodes u and v
    int mh_score(const NodeID u, const NodeID v, const SignatureMatrix& signatures) {
        int matches = 0;
        for (int h_idx = 0; h_idx < H_FUNCS; ++h_idx) {
            if (signatures.at(u).at(h_idx) == signatures.at(v).at(h_idx)) {
                matches++;
            }
        }
        return matches;
    }

    CandidatePairSet generate_candidates(const Graph& graph, const int k) {
        const size_t n = graph.size();
        SignatureMatrix signatures(n, std::vector<int>(H_FUNCS));

        // Line 1: Initialize h hash functionsLine
        // Line 2: Compute MinHash
        compute_minhashes(graph, signatures);

        // Line 3: Initialize a candidate pair set
        CandidatePairSet CP;

        // Line 4: For each node u in all nodes
        for (NodeID u = 0; u < static_cast<int>(n); ++u) {
            if (graph.at(u).empty()) continue;

            const std::vector<NodeID>& neighbors = graph.at(u);
            std::unordered_set<NodeID> two_hop_set;

            // Line 5: Determine subset S size (sample b nodes)
            const int num_to_sample = std::min(B_SAMPLE, static_cast<int>(neighbors.size()));

            // Line 6 (Part A): Let 2Hop <- N_u (include all immediate neighbors)
            for (NodeID one_hop : neighbors) if (one_hop > u) two_hop_set.insert(one_hop);

            // Line 6 (Part B): 2Hop <- Union of N_w for w in S (include all neighbors of the b sampled nodes in S)
            for (int i = 0; i < num_to_sample; ++i) {
                for (const NodeID one_hop = neighbors.at(i); const NodeID two_hop : graph.at(one_hop)) {
                    if (two_hop > u) two_hop_set.insert(two_hop);
                }
            }

            phmap::btree_set<std::pair<int, NodeID>> top_k;

            // Line 7: For each neighboring node in 2Hop
            for (const NodeID v : two_hop_set) {
                // Line 8: MinHash-based scoring (Equation 5)
                int score = mh_score(u, v, signatures);

                // Line 9: Select k nodes with highest mh(u,v)
                if (top_k.size() < k) top_k.emplace(score, v);
                else if (score > top_k.begin()->first) {
                    top_k.erase(top_k.begin());
                    top_k.emplace(score, v);
                }
            }

            // Line 10: Put (u, v) into CP for each selected v
            for (const auto&[_, v] : top_k) {
                if (u != v) CP.insert(u < v ? std::make_pair(u, v) : std::make_pair(v, u));
            }
        }
        return CP;
    }
}