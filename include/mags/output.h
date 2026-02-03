#ifndef MAGS_REWRITE_OUTPUT_H
#define MAGS_REWRITE_OUTPUT_H
#include "Partition.h"
#include "candidate_generation.h"

#include <utility>
#include <vector>

#include "types.h"

namespace mags::out {

struct Representation {
  std::vector<std::pair<NodeID, NodeID>> super_edges;
  std::vector<std::pair<NodeID, NodeID>> plus_corrections;
  std::vector<std::pair<NodeID, NodeID>> minus_corrections;
  Graph summary_graph;

  // Proper constructor for the return statement
  Representation(std::vector<std::pair<NodeID, NodeID>> se,
                 std::vector<std::pair<NodeID, NodeID>> pc,
                 std::vector<std::pair<NodeID, NodeID>> mc, const size_t n)
      : super_edges(std::move(se)), plus_corrections(std::move(pc)),
        minus_corrections(std::move(mc)) {
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

Representation output(const Graph &graph, const Partition &p);

} // namespace mags::out

#endif // MAGS_REWRITE_OUTPUT_H