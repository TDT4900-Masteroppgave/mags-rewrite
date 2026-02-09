
#ifndef MAGS_REWRITE_SUPER_NODE_SET_H
#define MAGS_REWRITE_SUPER_NODE_SET_H

#include "DisjointSetUnion.h"

#include <vector>

#include "types.h"

#include <parallel_hashmap/btree.h>
#include <parallel_hashmap/phmap.h>

#ifdef UNIT_TESTING
#include <gtest/gtest_prod.h>
#endif

using namespace mags;

class SuperNodeSet {
  DisjointSetUnion dsu;
  // stores a map, for each vertex, containing neighbor to the node, number
  // of edges between node and neighbor
  AllEdgeCounts edge_counts;

public:
  explicit SuperNodeSet(const Graph &graph);

  NodeID get_super_node(NodeID u) const;
  SuperNodes get_super_nodes() const;
  SuperNodeMembers get_super_node_members() const;
  EdgeCounts get_neighbor_edge_counts(NodeID u) const;
  EdgeCounts get_neighbor_edge_counts(NodeID u, NodeID v) const;
  int get_cartesian_product(NodeID u, NodeID v) const;
  static int get_cartesian_product(NodeID u, NodeID v, int u_edges,
                                   int v_edges);
  double get_cost(NodeID u) const;
  double get_merge_cost(NodeID u, NodeID v) const;
  static int get_unique_edges(NodeID u, NodeID v, int num_raw_edges);

  void merge(NodeID u, NodeID v);
  double saving(NodeID u, NodeID v) const;

#ifdef UNIT_TESTING
  [[nodiscard]] DisjointSetUnion get_dsu() const;
#endif

private:
  double accumulate_cost(
      NodeID u, int num_vertices_u,
      const phmap::flat_hash_map<int, int> &neighbor_edge_counts) const;
  void update_neighbor_edge_counts(NodeID u, NodeID v);

#ifdef UNIT_TESTING
  FRIEND_TEST(SuperNodeSetTest, UniqueEdges_SameNodeHalves);
  FRIEND_TEST(SuperNodeSetTest, UniqueEdges_DifferentNodesUnchanged);

  FRIEND_TEST(SuperNodeSetTest, AccumulateCost_WithInternalEdges);
  FRIEND_TEST(SuperNodeSetTest, AccumulateCost_NoInternalEdges);

  FRIEND_TEST(SuperNodeSetTest, UpdateNeighborEdgeCounts_Basic);
  FRIEND_TEST(SuperNodeSetTest, UpdateNeighborEdgeCounts_WithSharedNbrs);
#endif
};
#endif // MAGS_REWRITE_SUPER_NODE_SET_H
