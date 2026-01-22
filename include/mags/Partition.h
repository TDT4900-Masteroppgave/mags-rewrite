#ifndef MAGS_REWRITE_PARTITION_H
#define MAGS_REWRITE_PARTITION_H

#include "vector"

#include "DisjointSetUnion.h"
namespace mags {

class Partition {
  DisjointSetUnion dsu;
  SuperNodes super_nodes;
  SuperNodeMembers members;
  OriginalEdgeCounts edge_counts;

public:
  explicit Partition(int n);
  void finalize(const Graph &graph);

  const DisjointSetUnion &get_dsu() const;
  const SuperNodes &get_super_nodes() const;
  const SuperNodeMembers &get_members() const;
  const OriginalEdgeCounts &get_edge_counts() const;

  long long get_cartesian_product(NodeID u, NodeID v) const;
  NodeID find(NodeID u) const;
  void merge(NodeID u, NodeID v);
};

} // namespace mags

#endif // MAGS_REWRITE_PARTITION_H