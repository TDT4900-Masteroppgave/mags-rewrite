#ifndef MAGS_REWRITE_MAGS_TYPES_H
#define MAGS_REWRITE_MAGS_TYPES_H

#include <parallel_hashmap/phmap.h>
#include <vector>

namespace mags {
    using NodeID = int;
    using NodePair = std::pair<NodeID, NodeID>;
    using Graph = std::vector<std::vector<NodeID>>;
    using SuperNodes = std::vector<NodeID>;
    using SuperNodeMembers = std::vector<std::vector<NodeID>>;
    using EdgeCounts = std::vector<phmap::flat_hash_map<NodeID, int>>;
    using CandidateSet = std::vector<phmap::flat_hash_map<int, double>>;
    using PriorityQueue = phmap::btree_set<std::pair<double, mags::NodePair>, std::greater<>>;
}

#endif