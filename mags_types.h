#ifndef MAGS_REWRITE_MAGS_TYPES_H
#define MAGS_REWRITE_MAGS_TYPES_H

#include <utility>
#include <vector>

namespace mags {
    using NodeID = int;
    using CandidatePair = std::pair<NodeID, NodeID>;
    using CandidateSet = std::vector<CandidatePair>;
    using Graph = std::vector<std::vector<NodeID>>;
}

#endif //MAGS_REWRITE_MAGS_TYPES_H