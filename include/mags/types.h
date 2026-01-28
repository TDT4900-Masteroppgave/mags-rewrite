#ifndef MAGS_REWRITE_MAGS_TYPES_H
#define MAGS_REWRITE_MAGS_TYPES_H

#include <parallel_hashmap/phmap.h>
#include <vector>

namespace mags {
    using NodeID = int;
    using Graph = std::vector<std::vector<NodeID>>;
    using SuperNodes = std::vector<NodeID>;
    using SuperNodeMembers = std::vector<std::vector<NodeID>>;
    using OriginalEdgeCounts = std::vector<phmap::flat_hash_map<NodeID, int>>;
}

#endif