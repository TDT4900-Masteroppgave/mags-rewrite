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
using EdgeCounts = phmap::flat_hash_map<NodeID, int>;
using AllEdgeCounts = std::vector<phmap::flat_hash_map<NodeID, int>>;
using CandidateSet = std::vector<phmap::flat_hash_map<int, double>>;
using PriorityQueue =
    phmap::btree_set<std::pair<double, mags::NodePair>, std::greater<>>;

struct Representation {
    std::vector<std::pair<NodeID, NodeID>> super_edges;
    std::vector<std::pair<NodeID, NodeID>> plus_corrections;
    std::vector<std::pair<NodeID, NodeID>> minus_corrections;
    SuperNodeMembers group_members;
    Graph summary_graph;

    // Proper constructor for the return statement
    Representation(std::vector<std::pair<NodeID, NodeID>> se,
                   std::vector<std::pair<NodeID, NodeID>> pc,
                   std::vector<std::pair<NodeID, NodeID>> mc,
                   SuperNodeMembers members, const size_t n)
        : super_edges(std::move(se)), plus_corrections(std::move(pc)),
          minus_corrections(std::move(mc)), group_members(std::move(members)) {
        summary_graph.assign(n, {});

        for (const auto &[u, v] : super_edges) {
            summary_graph.at(u).push_back(v);
            if (u != v)
                summary_graph.at(v).push_back(u);
        }
    }

    [[nodiscard]] size_t get_total_cost() const {
        return super_edges.size() + plus_corrections.size() +
               minus_corrections.size();
    }
};

} // namespace mags

#endif