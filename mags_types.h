#ifndef MAGS_REWRITE_MAGS_TYPES_H
#define MAGS_REWRITE_MAGS_TYPES_H

#include "parallel_hashmap/phmap_fwd_decl.h"
#include <utility>
#include <vector>

namespace mags {
    using NodeID = int;
    using NodePair = std::pair<NodeID, NodeID>;
    using NodeSet = std::vector<NodePair>;
    using Graph = std::vector<std::vector<NodeID>>;
}
#endif //MAGS_REWRITE_MAGS_TYPES_H