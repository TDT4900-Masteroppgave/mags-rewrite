#ifndef MAGS_REWRITE_PARTITION_H
#define MAGS_REWRITE_PARTITION_H

#include "vector"

#include "DisjointSetUnion.h"
namespace mags {

class Partition {
  DisjointSetUnion dsu;
  SuperNodes super_nodes;
  SuperNodeMembers members;
  EdgeCounts edge_counts;

public:
  explicit Partition(size_t n);
  void finalize(const Graph &graph);

  const SuperNodes &get_super_nodes() const;
  const SuperNodeMembers &get_members() const;
  const EdgeCounts &get_edge_counts() const;

  long long get_cartesian_product(NodeID super_u, NodeID super_v) const;
  NodeID find_super_node(NodeID u) const;
  void merge(NodeID u, NodeID v);
};

} // namespace mags

#endif // MAGS_REWRITE_PARTITION_H