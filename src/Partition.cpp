#include "mags/Partition.h"
namespace mags {

Partition::Partition(const int n) : dsu(n) {}

void Partition::finalize(const Graph &graph) {
  const int n = static_cast<int>(graph.size());
  super_nodes.clear();
  members.assign(n, {});
  edge_counts.assign(n, {});

  // Identify roots and reserve memory
  for (NodeID u = 0; u < n; ++u) {
    if (NodeID root = dsu.find(u); root == u) {
      members.at(root).reserve(dsu.size(root));
      super_nodes.push_back(root);
    }
  }

  // Map every node to its super-node
  for (NodeID u = 0; u < n; ++u) {
    members.at(dsu.find(u)).push_back(u);
  }

  // Compute original edge counts
  for (NodeID u = 0; u < n; ++u) {
    for (const NodeID v : graph.at(u)) {
      // Only process an edge (u, v) once
      if (u <= v) {
        NodeID root_u = dsu.find(u);
        NodeID root_v = dsu.find(v);
        if (root_u > root_v)
          std::swap(root_u, root_v);
        edge_counts.at(root_u)[root_v]++;
      }
    }
  }
}

const DisjointSetUnion &Partition::get_dsu() const { return dsu; }

const SuperNodes &Partition::get_super_nodes() const { return super_nodes; }

const SuperNodeMembers &Partition::get_members() const { return members; }

const OriginalEdgeCounts &Partition::get_edge_counts() const {
  return edge_counts;
}

long long Partition::get_cartesian_product(const NodeID u,
                                           const NodeID v) const {
  const NodeID root_u = dsu.find(u);
  const NodeID root_v = dsu.find(v);

  const auto size_u = static_cast<long long>(members.at(root_u).size());
  if (root_u == root_v) {
    return size_u * (size_u - 1) / 2;
  }
  return size_u * static_cast<long long>(members.at(root_v).size());
}

NodeID Partition::find(const NodeID u) const { return dsu.find(u); }

void Partition::merge(const NodeID u, const NodeID v) { dsu.unite(u, v); }

} // namespace mags