#include "mags/SuperNodeSet.h"
#include "mags/types.h"

#include <ranges>
#include <algorithm>

using namespace mags;

SuperNodeSet::SuperNodeSet(const Graph &graph) : dsu(graph.size()) {
  // initializes edge count for between each node and their neighbor to 1,
  // no entry is created for nodes that does not have any neighbor
  edge_counts.resize(graph.size());
  for (int u = 0; u < static_cast<int>(graph.size()); u++) {
    for (int nbr : graph.at(u)) {
      // NOTE: This assumes simple graph (no parallel edges) initially.
      edge_counts[u][nbr] = 1;
    }
  }
}

NodeID SuperNodeSet::get_super_node(const NodeID u) const {
  return dsu.find(u);
}

SuperNodes SuperNodeSet::get_super_nodes() const {
  SuperNodes super_nodes;
  super_nodes.reserve(dsu.get_parents_size());
  for (NodeID u = 0; u < static_cast<int>(dsu.get_parents_size()); ++u) {
    NodeID root = dsu.find(u);
    if (root == u) {
      super_nodes.push_back(root);
    }
  }

  return super_nodes;
}

SuperNodeMembers SuperNodeSet::get_super_node_members() const {
  size_t n = dsu.get_parents_size();
  SuperNodeMembers members(n);

  for (NodeID u = 0; u < static_cast<int>(n); ++u) {
    NodeID root = dsu.find(u);
    members.at(root).push_back(u);
  }

  return members;
}

const EdgeCounts& SuperNodeSet::get_neighbor_edge_counts(const NodeID u) const {
  const NodeID u_super = get_super_node(u);
  return edge_counts[u_super];
}

EdgeCounts SuperNodeSet::get_neighbor_edge_counts(const NodeID u,
                                                  const NodeID v) const {
  const NodeID u_super = get_super_node(u);
  const NodeID v_super = get_super_node(v);

  // initializes the edge counts for w to the edge counts for u
  EdgeCounts w_neighbor_edge_counts = get_neighbor_edge_counts(u_super);

  // add the edge counts for v to w
  for (const auto &[nbr, num_nbr_edges] : get_neighbor_edge_counts(v_super)) {
    w_neighbor_edge_counts[nbr] += num_nbr_edges;
  }

  // Handles edges that used to exist between u and v.
  if (w_neighbor_edge_counts.contains(v_super)) {
    // stores all edges between u and v in the entry u i.e. as internal edges
    w_neighbor_edge_counts[u_super] += w_neighbor_edge_counts[v_super];
    w_neighbor_edge_counts.erase(v_super);
  }

  return w_neighbor_edge_counts;
}

int SuperNodeSet::get_unique_edges(NodeID u, NodeID v, int num_raw_edges) {
  // if u == v, each edge inside the same component is seen from both endpoints
  return (u == v) ? (num_raw_edges / 2) : num_raw_edges;
}

int SuperNodeSet::get_cartesian_product(const NodeID u, const NodeID v) const {
  if (u == v) {
    // uses the formula for number of unique pair of nodes in one single group
    return dsu.size(u) * (dsu.size(u) - 1) / 2;
  } else {
    // uses the formula for number of unique pair of nodes between two groups
    return dsu.size(u) * dsu.size(v);
  }
}

int SuperNodeSet::get_cartesian_product(const NodeID u, const NodeID v,
                                        const int num_vertices_u,
                                        const int num_vertices_v) {
  /*
      Getter for the cartesian product between two nodes.
      The cartesian product between two nodes corresponds to all possible
     combinations of edges.
  */

  if (u == v) {
    // uses the formula for number of unique pair of nodes in one single group
    return num_vertices_u * (num_vertices_u - 1) / 2;
  } else {
    // uses the formula for number of unique pair of nodes between two groups
    return num_vertices_u * num_vertices_v;
  }
}

double
SuperNodeSet::accumulate_cost(const NodeID u, const int num_vertices_u,
                              const EdgeCounts &neighbor_edge_counts) const {
  double total_cost = 0.0;

  // iterates over a map containing neighbors to the node and the number of
  // edges between the node and the neighbor
  for (const auto &[neighbor, num_nbr_edges] : neighbor_edge_counts) {
    const int cartesian_product =
        get_cartesian_product(u, neighbor, num_vertices_u, dsu.size(neighbor));
    const int unique_nbr_edges = get_unique_edges(u, neighbor, num_nbr_edges);

    // cart = 1, unique = 1, min(1-1+1=1, 1)=1
    total_cost +=
        std::min(cartesian_product - unique_nbr_edges + 1, unique_nbr_edges);
  }

  return total_cost;
}

double SuperNodeSet::get_cost(NodeID u) const {
  // Uses the optimized get_neighbor_edge_counts returning const reference
  return accumulate_cost(u, dsu.size(u), get_neighbor_edge_counts(u));
}

double SuperNodeSet::get_merge_cost(const NodeID u, const NodeID v) const {
  const NodeID u_super = get_super_node(u);
  const NodeID v_super = get_super_node(v);

  // If same node, just return its current cost (merging with self is no-op in terms of set logic,
  // but usually this function is called for distinct nodes)
  if (u_super == v_super) return get_cost(u_super);

  const int merged_num_vertices = dsu.size(u_super) + dsu.size(v_super);
  const auto& u_edges = edge_counts[u_super];
  const auto& v_edges = edge_counts[v_super];

  double total_cost = 0.0;
  int internal_edges_count = 0;

  // Iterate over U's neighbors
  for (const auto& [nbr, count_u] : u_edges) {
      if (nbr == v_super) {
          // Edge between u and v becomes internal self-loop
          internal_edges_count += count_u;
          continue;
      }

      int total_count = count_u;

      // Check if nbr is also in V
      auto it_v = v_edges.find(nbr);
      if (it_v != v_edges.end()) {
          total_count += it_v->second;
      }

      if (nbr == u_super) {
          // Already a self-loop on U
          internal_edges_count += count_u;
          continue;
      }

      // Compute cost for this neighbor
      // Since nbr is neither u_super nor v_super, it is an external group.
      const int cartesian = get_cartesian_product(u_super, nbr, merged_num_vertices, dsu.size(nbr));
      // For external edges, unique check is just the count (get_unique_edges checks if node1==node2)
      // Here node1 is the merged node (virtual), node2 is nbr. They are different.
      const int unique = total_count;

      total_cost += std::min(cartesian - unique + 1, unique);
  }

  // Iterate over V's neighbors
  for (const auto& [nbr, count_v] : v_edges) {
      if (nbr == u_super) {
          // Edge between v and u. Already handled in U loop (nbr == v_super check was for u's neighbors).
          // Wait. U's map has entry 'v'. V's map has entry 'u'.
          // In U loop: if (nbr == v_super) -> internal_edges_count += count_u.
          // In V loop: if (nbr == u_super) -> internal_edges_count += count_v? NO.
          // We summed degrees.
          // If we have edge u-v. u has v (1). v has u (1).
          // We added 1 in U loop.
          // If we add 1 here, sum is 2.
          // Unique = 2/2 = 1. Correct.
          // So we MUST add it here too.
          internal_edges_count += count_v;
          continue;
      }

      if (nbr == v_super) {
         // Self loop on v
         internal_edges_count += count_v;
         continue;
      }

      // Check if processed in U's loop
      if (u_edges.contains(nbr)) {
          continue; // Already processed
      }

      // Process neighbor only in V
      const int cartesian = get_cartesian_product(u_super, nbr, merged_num_vertices, dsu.size(nbr));
      const int unique = count_v;
      total_cost += std::min(cartesian - unique + 1, unique);
  }

  // Handle internal edges (self-loops on the new merged node w)
  if (internal_edges_count > 0) {
      // The merged node w has internal_edges_count raw edges (sum of degrees for internal links).
      // Unique edges = raw / 2.
      // Cartesian product for self loop = w * (w-1) / 2.

      const int unique = internal_edges_count / 2;
      const int cartesian = merged_num_vertices * (merged_num_vertices - 1) / 2;

      total_cost += std::min(cartesian - unique + 1, unique);
  }

  return total_cost;
}

double SuperNodeSet::saving(NodeID u, NodeID v) const {
  double c_u = get_cost(u);
  double c_v = get_cost(v);
  double c_w = get_merge_cost(u, v);

  return (c_u + c_v - c_w) / (c_u + c_v);
}

void SuperNodeSet::update_neighbor_edge_counts(const NodeID u, const NodeID v) {
  // update the edge count to the merged node w (stored in u)
  // TODO: this step can be optimized because the saving step has already
  // performed the same calculation.

  const NodeID u_super = get_super_node(u);
  const NodeID v_super = get_super_node(v);

  if (u_super == v_super) {
    return;
  }

  EdgeCounts w_neighbor_edge_counts =
      get_neighbor_edge_counts(u_super, v_super);
  edge_counts[u_super] = std::move(w_neighbor_edge_counts);

  // delete v as neighbor
  for (const auto nbr : edge_counts[v_super] | std::views::keys) {
    edge_counts[nbr].erase(v_super);
  }

  // delete v as an entry
  edge_counts[v_super].clear();

  // update the edge count where w (stored in u) is a nbr
  for (auto [nbr, num_nbr_edges] : edge_counts[u_super]) {
    if (nbr != u_super) {
      edge_counts[nbr][u_super] = num_nbr_edges;
    }
  }
}

void SuperNodeSet::merge(const NodeID u, const NodeID v) {
  // replaces u and v in neighbor_edge_counts to w
  update_neighbor_edge_counts(u, v);
  dsu.unite(u, v);
}

#ifdef UNIT_TESTING
[[nodiscard]] DisjointSetUnion SuperNodeSet::get_dsu() const { return dsu; }
#endif
