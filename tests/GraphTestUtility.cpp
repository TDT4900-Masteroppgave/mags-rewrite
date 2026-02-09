#include "GraphTestUtility.h"
#include "mags/candidate_generation.h"
#include "mags/output.h"
#include "mags/types.h"

#include <fstream>
#include <filesystem>
#include <gtest/gtest.h>

namespace mags::test {

void GraphTestUtility::SetUp() {
  cg::SEED = 233;

  diamond = {{1, 2}, {0, 3}, {0, 3}, {1, 2}};
  triangle = {{1, 2}, {0, 2}, {0, 1}};
  path = {{1}, {0, 2}, {1, 3}, {2}};
  isolated = {{1}, {0}, {}, {}, {}};
  star = create_star_graph();
  ladder = create_ladder_graph();
  clique = create_clique_graph();

  tmp_file_name = "test_graph.txt";

  for (auto *g :
       {&diamond, &triangle, &star, &path, &ladder, &isolated, &clique}) {
    for (auto &neighbors : *g) {
      std::ranges::sort(neighbors);
    }
  }
}

Graph GraphTestUtility::create_ladder_graph() {
  Graph g;
  g.assign(15, {});

  g.at(0) = {10, 11, 12, 14, 14};
  for (const int nbr : g.at(0))
    g.at(nbr).push_back(0);

  for (int i = 1; i <= 5; ++i) {
    for (int j = 0; j < i; ++j) {
      constexpr int shared_nbr = 10;
      g.at(i).push_back(shared_nbr);
      g.at(shared_nbr).push_back(i);
    }
  }
  return g;
}

Graph GraphTestUtility::create_star_graph() {
  Graph g(11);
  for (int i = 0; i <= 10; ++i) {
    g.at(0).push_back(i);
    g.at(i).push_back(0);
  }
  return g;
}

Graph GraphTestUtility::create_clique_graph() {
  Graph g(4);
  for (int i = 0; i < g.size(); ++i) {
    for (int j = i + 1; j < g.size(); ++j) {
      g.at(i).push_back(j);
      g.at(j).push_back(i);
    }
  }
  return g;
}

Graph GraphTestUtility::reconstruct_graph(const Representation &rep,
                                          const size_t n) {
  Graph reconstructed(n);

  // 1. Expand Super-edges
  for (const auto &[u_root, v_root] : rep.super_edges) {
    const auto &nodes_u = rep.group_members.at(u_root);
    const auto &nodes_v = rep.group_members.at(v_root);

    for (NodeID node_u : nodes_u) {
      for (NodeID node_v : nodes_v) {
        if (u_root == v_root && node_u >= node_v)
          continue; // Internal self-loop logic

        reconstructed[node_u].push_back(node_v);
        reconstructed[node_v].push_back(node_u);
      }
    }
  }

  // 2. Add Plus Corrections
  for (const auto &[u, v] : rep.plus_corrections) {
    reconstructed[u].push_back(v);
    reconstructed[v].push_back(u);
  }

  // 3. Remove Minus Corrections
  for (const auto &[u, v] : rep.minus_corrections) {
    auto &adj_u = reconstructed[u];
    std::erase(adj_u, v);

    auto &adj_v = reconstructed[v];
    std::erase(adj_v, u);
  }

  // Sort neighbors for comparison
  for (auto &neighbors : reconstructed) {
    std::ranges::sort(neighbors);
  }
  return reconstructed;
}

size_t GraphTestUtility::get_edge_count(const Graph &graph) {
  size_t edge_count = 0;
  for (const auto &neighbors : graph) {
    edge_count += neighbors.size();
  }

  edge_count /= 2;

  return edge_count;
}

void GraphTestUtility::write_tmp_file(const std::string &content) const {
  std::ofstream outfile(tmp_file_name);
  outfile << content;
  outfile.close();
}
} // namespace mags::test
