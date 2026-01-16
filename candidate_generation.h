#ifndef MAGS_REWRITE_CANDIDATE_GENERATION_H
#define MAGS_REWRITE_CANDIDATE_GENERATION_H

#include "types.h"
#include <vector>

namespace mags::cg {
    constexpr int H_FUNCS = 40;
    constexpr int B_SAMPLE = 5;
    inline uint64_t SEED = 2333;

    using CandidatePairSet = std::set<std::pair<NodeID, NodeID>>;
    using SignatureMatrix = std::vector<std::vector<int>>;

    void compute_minhashes(const Graph& graph, SignatureMatrix& signatures);
    int mh_score(NodeID u, NodeID v, const SignatureMatrix& signatures);
    CandidatePairSet generate_candidates(const Graph& graph, int k);
}

#endif //MAGS_REWRITE_CANDIDATE_GENERATION_H