#ifndef MAGS_REWRITE_MAGS_TYPES_H
#define MAGS_REWRITE_MAGS_TYPES_H

#include <random>
#include <set>
#include <vector>

namespace mags {
    using NodeID = int;
    using Graph = std::vector<std::vector<NodeID>>;
}

#endif