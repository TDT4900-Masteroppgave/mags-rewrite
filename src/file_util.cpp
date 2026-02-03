#include "mags/file_util.h"

#include <fstream>
#include <iostream>

namespace mags::io {

Graph read_from_file(const std::string &path) {
  std::ifstream fin(path);
  if (!fin.is_open()) {
    std::cerr << "Error: Cannot open file " << path << std::endl;
    return {};
  }

  std::vector<std::pair<NodeID, NodeID>> edges;
  int u, v;
  int max_node_id = -1;

  // Line 1: Read all edges into memory
  while (fin >> u >> v) {
    edges.emplace_back(u, v);
    max_node_id = std::max({max_node_id, u, v});
  }
  fin.close();

  // Line 2: Initialize Graph with n nodes
  const int n = max_node_id + 1;
  Graph graph(n);

  // Line 3: Add edges to adjacency lists
  // Note: This does NOT ensure undirectedness or clean data yet;
  // that will be handled by your mags::preprocess::clean_graph.
  for (const auto& [from, to] : edges) {
    if (from >= 0 && from < n) {
      graph.at(from).push_back(to);
    }
  }

  return graph;
}
} // namespace mags::io