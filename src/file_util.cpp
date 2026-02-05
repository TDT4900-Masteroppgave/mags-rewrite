#include "mags/file_util.h"

#include <filesystem>
#include <fstream>
#include <iostream>

namespace mags::io {

Graph read_from_file(const std::string &path) {
  std::ifstream fin(path);
  if (!fin.is_open()) {
    std::cerr << "Error: Cannot open file " << path << std::endl;
    return {};
  }

  int u, v;
  int max_node_id = -1;

  while (fin >> u >> v) {
    if (u > max_node_id)
      max_node_id = u;
    if (v > max_node_id)
      max_node_id = v;
  }

  fin.clear();
  fin.seekg(0);

  Graph graph(max_node_id + 1);

  while (fin >> u >> v) {
    graph.at(u).push_back(v);
  }

  return graph;
}
} // namespace mags::io