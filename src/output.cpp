#include "mags/output.h"

#include "mags/SuperNodeSet.h"

namespace mags::out {

namespace {

void find_minus_corrections(const Graph &graph,
                            const std::vector<NodeID> &u_members,
                            const std::vector<NodeID> &v_members,
                            std::vector<std::pair<NodeID, NodeID>> &minus_out) {
  for (const NodeID u : u_members) {
    const std::vector<NodeID> &neighbors = graph.at(u);
    int actual_idx = 0;

    for (const NodeID v : v_members) {
      // Avoid self-loops and double-counting undirected edges
      if (u >= v)
        continue;

      // Fast-forward actual_neighbors to find node_v
      while (actual_idx < neighbors.size() && neighbors[actual_idx] < v) {
        actual_idx++;
      }

      // If we didn't find node_v in the actual neighbors,
      // it's a minus correction
      if (actual_idx == neighbors.size() || neighbors[actual_idx] != v) {
        minus_out.emplace_back(u, v);
      }
    }
  }
}

void find_plus_corrections(const Graph &graph,
                           const std::vector<NodeID> &u_members,
                           const std::vector<NodeID> &v_members,
                           std::vector<std::pair<NodeID, NodeID>> &plus_out) {

  for (const NodeID u : u_members) {
    const std::vector<NodeID> &neighbors = graph.at(u);
    int neighbors_idx = 0;
    int v_members_idx = 0;

    // Standard two-pointer intersection logic for sorted lists
    while (neighbors_idx < neighbors.size() &&
           v_members_idx < v_members.size()) {
      const NodeID neighbor_v = neighbors[neighbors_idx];
      const NodeID target_v = v_members[v_members_idx];

      if (neighbor_v < target_v) {
        neighbors_idx++;
      } else if (neighbor_v > target_v) {
        v_members_idx++;
      } else {
        // We found an edge (u, target_v)
        // Since neighbor_v == target_v, it's a plus correction

        // Ensure undirected symmetry
        if (u < target_v) {
          plus_out.emplace_back(u, target_v);
        }
        neighbors_idx++;
        v_members_idx++;
      }
    }
  }
}
} // namespace

Representation output(const Graph &graph, const SuperNodeSet &p) {
  // Line 1: Initialize Super-Edges and Corrections
  std::vector<std::pair<NodeID, NodeID>> super_edges;
  std::vector<std::pair<NodeID, NodeID>> plus_corrections;
  std::vector<std::pair<NodeID, NodeID>> minus_corrections;

  const auto members = p.get_super_node_members();

  // Line 2: For each super-node u in P
  for (NodeID super_u : p.get_super_nodes()) {
    // Line 2: For each neighboring super-node v to u
    for (const auto &[super_v, raw_count] :
         p.get_neighbor_edge_counts(super_u)) {
      // Process each pair {u, v} only once
      if (super_u > super_v)
        continue;

      // TODO: Evaluate E_uv count, is it correctly used with unique edges?
      const int e_uv_count =
          SuperNodeSet::get_unique_edges(super_u, super_v, raw_count);
      // Line 4: If E_uv > (Pi_uv + 1) / 2
      if (const long long pi_uv = p.get_cartesian_product(super_u, super_v);
          e_uv_count > (pi_uv + 1) / 2) {
        // Line 5: Add super-edge
        super_edges.emplace_back(super_u, super_v);
        // Line 5: Add minus correction
        find_minus_corrections(graph, members[super_u], members[super_v],
                               minus_corrections);
      } else {
        // Line 6: Add plus correction
        find_plus_corrections(graph, members[super_u], members[super_v],
                              plus_corrections);
      }
    }
  }

  // Line 7: Return representation with summary graph and corrections
  return Representation(std::move(super_edges), std::move(plus_corrections),
          std::move(minus_corrections), members, graph.size(), graph);
}

} // namespace mags::out