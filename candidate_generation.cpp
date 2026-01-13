#include "candidate_generation.h"

#include <set>

mags::CandidateSet generate_candidates(mags::Graph const& graph) {
    mags::CandidateSet candidates;
    const size_t n = static_cast<mags::NodeID>(graph.size());

    for (mags::NodeID u = 0; u < n; u++) {
        std::set<mags::NodeID> potential_candidates;

        for (const mags::NodeID v_1hop : graph[u]) {
            if (v_1hop > u) potential_candidates.insert(v_1hop);

            for (mags::NodeID v_2hop : graph[v_1hop]) {
                if (v_2hop > u) potential_candidates.insert(v_2hop);
            }
        }

        for (mags::NodeID v : potential_candidates) {
            candidates.emplace_back(u, v);
        }
    }

    return candidates;
}
