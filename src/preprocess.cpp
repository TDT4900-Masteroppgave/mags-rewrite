#include "mags/preprocess.h"

namespace mags::preprocess {

Graph clean_graph(Graph &graph) {
  const int n = static_cast<int>(graph.size());

  for (NodeID u = 0; u < n; ++u) {
    // Ensure undirectedness@
    std::vector<NodeID> neighbors = graph.at(u);
    for (const NodeID v : neighbors) {
      if (v != u) {
        graph.at(v).push_back(u);
      }
    }
  }

  for (NodeID u = 0; u < n; ++u) {
    auto &neighbors = graph.at(u);
    // Remove self-loops
    std::erase_if(neighbors, [u](const NodeID v) { return u == v; });
    // Sort neighbors
    std::ranges::sort(neighbors);
    // Remove duplicates
    auto [first, last] = std::ranges::unique(neighbors);
    neighbors.erase(first, last);
  }

  return graph;
}

} // namespace mags::preprocess