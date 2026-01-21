#ifndef MAGS_REWRITE_CANDIDATE_GENERATION_H
#define MAGS_REWRITE_CANDIDATE_GENERATION_H

#include <unordered_set>

#include "types.h"

#include <parallel_hashmap/btree.h>
#include <set>
#include <vector>

namespace mags::cg {
    constexpr int H_FUNCS = 40;
    constexpr int B_SAMPLE = 5;
    inline uint64_t SEED = 2333;

    using CandidatePairSet = std::set<std::pair<NodeID, NodeID>>;
    using SignatureMatrix = std::vector<std::vector<int>>;

    CandidatePairSet generate_candidates(const Graph& graph, int k);

    namespace detail {
        void compute_minhashes(const Graph& graph, SignatureMatrix& signatures);
        int mh_score(NodeID u, NodeID v, const SignatureMatrix& signatures);
        void get_two_hop_neighbors(const Graph& graph, NodeID u, int b, std::unordered_set<NodeID>& two_hop_neighbors);
        void get_top_k_candidate_pairs(
            NodeID u,
            int k,
            const SignatureMatrix& signatures,
            const std::unordered_set<NodeID>& two_hop_neighbors,
            phmap::btree_set<std::pair<int, NodeID>>& top_k);
    }
}

#endif //MAGS_REWRITE_CANDIDATE_GENERATION_H