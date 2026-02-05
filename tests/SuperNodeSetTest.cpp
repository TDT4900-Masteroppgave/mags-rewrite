#include <gtest/gtest.h>
#include <mags/SuperNodeSet.h>

using namespace mags;

TEST(SuperNodeSetTest, Initialization) {
  const Graph graph = {{1}, {0, 2}, {1}};
  SuperNodeSet s(graph);
  EXPECT_EQ(s.get_super_nodes().size(), graph.size());
  for (const NodeID root : s.get_super_nodes()) {
    for (const auto &[nbr, num_edges] : s.get_neighbor_edge_counts(root)) {
      EXPECT_EQ(s.get_neighbor_edge_counts(root).at(nbr), 1);
    }
  }
}

TEST(SuperNodeSetTest, NeighborEdgeCounts_Basic) {
  // verifies that neighbor_edge_counts[u_super] contains the expected
  // neighbor per edge count map
  const Graph g = {{1, 2}, {0}, {0}};
  SuperNodeSet super_nodes(g);

  auto neighbor_edge_counts_0 = super_nodes.get_neighbor_edge_counts(0);

  EXPECT_EQ(neighbor_edge_counts_0.size(), 2);
  EXPECT_EQ(neighbor_edge_counts_0.at(1), 1);
  EXPECT_EQ(neighbor_edge_counts_0.at(2), 1);
}

TEST(SuperNodeSetTest, NeighborEdgeCounts_UV_NoInternalEdge) {
  const Graph g = {{1, 2}, {0}, {0}};
  SuperNodeSet super_nodes(g);

  auto w_neighbor_edge_counts = super_nodes.get_neighbor_edge_counts(1, 2);

  // Expect: neighbor 0 appears twice (one edge from 1->0 and one from 2->0)
  EXPECT_EQ(w_neighbor_edge_counts.size(), 1);
  EXPECT_TRUE(w_neighbor_edge_counts.contains(0));
  EXPECT_EQ(w_neighbor_edge_counts.at(0), 2);
}

TEST(SuperNodeSetTest, NeighborEdgeCounts_UV_WithInternalEdge) {
  const Graph g = {{1, 2}, {0, 2}, {0, 1}};
  // const Graph g = {{1, 2}, {0, 2}, {0, 1}};
  SuperNodeSet super_nodes(g);

  // g[0]= {(1, 1), (2, 1)}, g[1] = {(0, 1), (2, 1)}
  auto w_counts = super_nodes.get_neighbor_edge_counts(0, 1);
  // g[0]= {(0, 1), (1, 1), (2, 2)}, g[1] = {(0, 1)}
  // g[0]= {(0, 2), (2, 2)}}

  // v_super=1 should be erased from the resulting map
  EXPECT_FALSE(w_counts.contains(1));

  // The edge(s) between u and v get stored as "internal edges" under u_super
  // (key 0). Here there is one 0-1 edge, and counts(1) also contributes one
  // 1->0 edge, so w[0] ends up being 2 as derived above.
  EXPECT_TRUE(w_counts.contains(0));
  EXPECT_EQ(w_counts.at(0), 2);

  // // The shared neighbor 2 should have count 2 (one from 0->2 and one from
  // 1->2)
  EXPECT_TRUE(w_counts.contains(2));
  EXPECT_EQ(w_counts.at(2), 2);

  EXPECT_EQ(w_counts.size(), 2);
}

TEST(SuperNodeSetTest, NeighborEdgeCounts_UV_WithInternalEdge2) {
  // Graph:
  // 0 -- 1 -- 2
  const Graph g = {{1}, {0, 2}, {1}};

  SuperNodeSet sns(g);

  const NodeID u_super = sns.get_super_node(0);
  const NodeID v_super = sns.get_super_node(1);
  ASSERT_NE(u_super, v_super);

  auto u_map = sns.get_neighbor_edge_counts(u_super, v_super);

  EXPECT_FALSE(u_map.contains(v_super)); // v removed
  EXPECT_TRUE(u_map.contains(u_super));  // internal entry for 0 with count 2
  EXPECT_EQ(u_map.at(u_super), 2);
  EXPECT_TRUE(u_map.contains(2)); // shared neighbor
  EXPECT_EQ(u_map.at(2), 1);
}

TEST(SuperNodeSetTest, UniqueEdges_SameNodeHalves) {
  const Graph g = {{}};
  SuperNodeSet super_nodes(g);

  EXPECT_EQ(super_nodes.get_unique_edges(0, 0, 0), 0);
  EXPECT_EQ(super_nodes.get_unique_edges(0, 0, 2), 1);
  EXPECT_EQ(super_nodes.get_unique_edges(0, 0, 4), 2);
}

TEST(SuperNodeSetTest, UniqueEdges_DifferentNodesUnchanged) {
  const Graph g = {{}, {}};
  SuperNodeSet super_nodes(g);

  EXPECT_EQ(super_nodes.get_unique_edges(0, 1, 3), 3); // u != v
}

TEST(SuperNodeSetTest, SimpleCatesianProduct) {
  const Graph graph = {{1}, {0}, {3}, {2}};
  SuperNodeSet s(graph);
  s.merge(0, 1); // Super-Node A size 2
  s.merge(2, 3); // Super-Node B size 2

  // |Pi_uv| = 2 * 2 = 4
  EXPECT_EQ(s.get_cartesian_product(0, 2), 4);
}

TEST(SuperNodeSetTest, CartesianPrdouctSelfLoop) {
  const Graph graph = {{1}, {0}, {2}};
  SuperNodeSet s(graph);
  s.merge(0, 1);
  s.merge(1, 2); // Super-Node size 3

  EXPECT_EQ(s.get_cartesian_product(0, 0), 3);
}

TEST(SuperNodeSetTest, CartesianProductSingleNode) {
  const Graph graph = {{0}};
  const SuperNodeSet s(graph);
  EXPECT_EQ(s.get_cartesian_product(0, 0), 0);
}

TEST(SuperNodeSetTest, CartesianProduct_SameNode) {
  const Graph g = {{}};
  SuperNodeSet super_nodes(g);

  // u == v => n*(n-1)/2, note that num_vertices_v is ignored in the u==v branch
  EXPECT_EQ(super_nodes.get_cartesian_product(0, 0, 0, -1), 0);
  EXPECT_EQ(super_nodes.get_cartesian_product(0, 0, 1, -1), 0);
  EXPECT_EQ(super_nodes.get_cartesian_product(0, 0, 2, -1), 1);
  EXPECT_EQ(super_nodes.get_cartesian_product(0, 0, 5, -1), 10);
}

TEST(SuperNodeSetTest, CartesianProduct_DifferentNodes) {
  const Graph g = {{}, {}};
  SuperNodeSet super_nodes(g);

  EXPECT_EQ(super_nodes.get_cartesian_product(0, 1, 0, 5), 0);
  EXPECT_EQ(super_nodes.get_cartesian_product(0, 1, 3, 4), 12);
  EXPECT_EQ(super_nodes.get_cartesian_product(1, 0, 7, 1), 7);
}

TEST(SuperNodeSetTest, AccumulateCost_WithInternalEdges) {
  const Graph g = {{}, {}}; // 2 nodes
  const SuperNodeSet super_nodes(g);

  phmap::flat_hash_map<int, int> neighbor_edge_counts;
  neighbor_edge_counts.emplace(
      0, 2); // internal edge: raw=2 => unique=1, cart=1 => min(1-1+1=1, 1)=1
  neighbor_edge_counts.emplace(
      1, 1); // normal edge: raw=1 => unique=1, cart=1 => min(1,1)=1

  const double cost = super_nodes.accumulate_cost(/*u*/ 0, /*num_vertices_u*/ 2,
                                                  neighbor_edge_counts);
  EXPECT_DOUBLE_EQ(cost, 2.0);
}

TEST(SuperNodeSetTest, AccumulateCost_NoInternalEdges) {
  const Graph g = {{}, {}, {}};
  SuperNodeSet super_nodes(g);

  // total_cost += min(cartesian - unique + 1, unique)
  // For u!=neighbor: cartesian = 1*1 = 1

  phmap::flat_hash_map<int, int> neighbor_edge_counts;
  neighbor_edge_counts.emplace(1, 1); // unique=1, cart=1 => min(1-1+1=1, 1)=1
  neighbor_edge_counts.emplace(2, 1); // unique=1, cart=1 => min(1-1+1=1, 1)=1

  const double cost = super_nodes.accumulate_cost(/*u*/ 0, /*num_vertices_u*/ 1,
                                                  neighbor_edge_counts);
  EXPECT_DOUBLE_EQ(cost, 2.0);
}

TEST(SuperNodeSetTest, GetCost_WithTwoDifferentNeighbors) {
  const Graph g = {
      {1, 2}, // 0 -> 1, 0 -> 2
      {},     // 1
      {}      // 2
  };
  SuperNodeSet super_nodes(g);

  double cost0 = super_nodes.get_cost(/*u=*/0);
  EXPECT_DOUBLE_EQ(cost0, 2.0);
}

TEST(SuperNodeSetTest, GetCost_WithSelfEdge) {
  const Graph g = {
      {0}, // 0 -> 0
  };
  SuperNodeSet super_nodes(g);
  //   num_vertices_u = 1
  //   unique=floor(1/2)=0, cart=1*0/2=0 => min(0-0+1=1, 0)=0

  double cost0 = super_nodes.get_cost(/*u=*/0);
  EXPECT_DOUBLE_EQ(cost0, 0.0);
}

TEST(SuperNodeSetGetCostTest, Cost_AfterMerge_NoChange) {
  const Graph g = {{1, 2}, {0}, {0}};

  SuperNodeSet super_nodes(g);

  // Sanity before merge
  // u=0: neighbors {1:1, 2:1} -> each contributes 1 -> total 2
  EXPECT_DOUBLE_EQ(super_nodes.get_cost(0), 2.0);
  // u=1: neighbor {0:1} -> contributes 1
  EXPECT_DOUBLE_EQ(super_nodes.get_cost(1), 1.0);
  // u=2: no neighbors -> cost 0
  EXPECT_DOUBLE_EQ(super_nodes.get_cost(2), 1.0);

  // Merge 0 and 1
  super_nodes.merge(0, 1);

  EXPECT_DOUBLE_EQ(super_nodes.get_cost(0), 2.0);

  EXPECT_DOUBLE_EQ(super_nodes.get_cost(2), 1.0);
}

TEST(SuperNodeSetGetCostTest, Cost_AfterMerge_WithChange) {
  const Graph g = {{2}, {2}, {0, 1}};

  SuperNodeSet super_nodes(g);

  // --- Sanity before merge ---
  EXPECT_DOUBLE_EQ(super_nodes.get_cost(0), 1.0);
  EXPECT_DOUBLE_EQ(super_nodes.get_cost(1), 1.0);
  EXPECT_DOUBLE_EQ(super_nodes.get_cost(2), 2.0);

  super_nodes.merge(0, 1);

  // cart = 2, unique = 2, min(2-2+1, 2) = 1
  EXPECT_DOUBLE_EQ(super_nodes.get_cost(0), 1.0);
  // node 1 should now refer to supernode 1 and hence has the same cost as 0
  EXPECT_DOUBLE_EQ(super_nodes.get_cost(1), 1.0);

  // cart = 2, unique = 2, min(2-2+1, 2) = 1
  EXPECT_DOUBLE_EQ(super_nodes.get_cost(2), 1.0);
}

TEST(SuperNodeSetTest, MergeCost_NoChange) {
  const Graph g = {{1, 2}, {0}, {0}};

  SuperNodeSet super_nodes(g);

  EXPECT_DOUBLE_EQ(super_nodes.get_merge_cost(0, 1), 2.0);
  EXPECT_DOUBLE_EQ(super_nodes.get_cost(2), 1.0);
}

TEST(SuperNodeSetGetCostTest, MergeCost_NotAffectingOtherNodes) {
  const Graph g = {{2}, {2}, {0, 1}};

  SuperNodeSet super_nodes(g);

  EXPECT_DOUBLE_EQ(super_nodes.get_merge_cost(0, 1), 1.0);
  EXPECT_DOUBLE_EQ(super_nodes.get_cost(2),
                   2.0); // a hypothetical merge does not affect nbr nodes
}

TEST(SuperNodeSetGetCostTest, MergeCost_DiffersFromBothOriginals) {
  // Graph:
  // 0 -- 1 -- 2
  // |         |
  // 4         3
  const Graph g = {{1, 4}, {0, 2, 3}, {1}, {1}, {0}};

  SuperNodeSet s(g);

  // Original costs
  EXPECT_DOUBLE_EQ(s.get_cost(0), 2.0); // neighbors {1,4}
  EXPECT_DOUBLE_EQ(s.get_cost(1), 3.0); // neighbors {0,2,3}

  // Hypothetical merge (0,1):
  // cart = 1, unique = 1, min(1-1+1=1, 1) = 1
  // cart = 2, unique = 1, min(2-1+1=2, 1) = 1
  // cart = 2, unique = 1, min(2-1+1=2, 1) = 1
  // cart = 2, unique = 1, min(2-1+1=2, 1) = 1
  // Total cw = 4
  const double cw = s.get_merge_cost(0, 1);
  EXPECT_DOUBLE_EQ(cw, 4.0);

  // Ensure it differs from both originals
  EXPECT_NE(cw, s.get_cost(0));
  EXPECT_NE(cw, s.get_cost(1));
}

TEST(SuperNodeSetSavingTest, Saving_Positive) {
  const Graph g = {{2}, {2}, {0, 1}};

  SuperNodeSet s(g);

  EXPECT_DOUBLE_EQ(s.get_cost(0), 1.0);
  EXPECT_DOUBLE_EQ(s.get_cost(1), 1.0);

  const double cw = s.get_merge_cost(0, 1);
  EXPECT_DOUBLE_EQ(cw, 1.0); // hypothetical merged-node cost

  const double sav = s.saving(0, 1);
  EXPECT_DOUBLE_EQ(sav, 0.5);

  // symmetry
  EXPECT_DOUBLE_EQ(s.saving(1, 0), sav);
}

TEST(SuperNodeSetSavingTest, Saving_Zero) {
  const Graph g = {
      {2}, // 0
      {3}, // 1
      {0}, // 2
      {1}  // 3
  };
  SuperNodeSet s(g);

  EXPECT_DOUBLE_EQ(s.get_cost(0), 1.0);
  EXPECT_DOUBLE_EQ(s.get_cost(1), 1.0);

  const double cw = s.get_merge_cost(0, 1);
  EXPECT_DOUBLE_EQ(cw, 2.0);

  const double sav = s.saving(0, 1);
  EXPECT_DOUBLE_EQ(sav, 0.0);
  EXPECT_DOUBLE_EQ(s.saving(1, 0), sav);
}

TEST(SuperNodeSetTest, UpdateNeighborEdgeCounts_Basic) {
  // Graph:
  // 0 -- 1 -- 2
  const Graph g = {{1}, {0, 2}, {1}};

  SuperNodeSet sns(g);

  const NodeID u_super = sns.get_super_node(0);
  const NodeID v_super = sns.get_super_node(1);
  ASSERT_NE(u_super, v_super);

  sns.update_neighbor_edge_counts(u_super, v_super);

  // Check the u_super neighbor map
  auto u_map = sns.get_neighbor_edge_counts(u_super);
  EXPECT_FALSE(u_map.contains(v_super)); // v removed
  EXPECT_TRUE(u_map.contains(u_super));  // internal entry for 0 with count 2
  EXPECT_EQ(u_map.at(u_super), 2);
  EXPECT_TRUE(u_map.contains(2)); // shared neighbor
  EXPECT_EQ(u_map.at(2), 1);

  // Check the map to neighbor 2
  auto n2_map = sns.get_neighbor_edge_counts(2);
  EXPECT_FALSE(n2_map.contains(v_super));
  EXPECT_TRUE(n2_map.contains(u_super));
  EXPECT_EQ(n2_map.at(u_super), 1);

  // Check that v_super map is cleared
  auto v_map = sns.get_neighbor_edge_counts(v_super);
  EXPECT_TRUE(v_map.empty());
}

TEST(SuperNodeSetTest, UpdateNeighborEdgeCounts_WithSharedNbrs) {
  // Graph:
  //   0
  //  / \
  // 1---2
  const Graph g = {
      {1, 2}, // 0
      {0, 2}, // 1
      {0, 1}  // 2
  };

  SuperNodeSet sns(g);
  const NodeID u_super = sns.get_super_node(0); // 0
  const NodeID v_super = sns.get_super_node(1); // 1
  ASSERT_NE(u_super, v_super);

  sns.update_neighbor_edge_counts(u_super, v_super);

  // Expected:
  // - v_super removed everywhere and cleared
  // - internal 0–1 counted under key 0: 2
  // - shared neighbor (2) gets combined count: 0->2 (1) + 1->2 (1) = 2
  auto u_map = sns.get_neighbor_edge_counts(u_super);
  EXPECT_FALSE(u_map.contains(v_super));
  EXPECT_EQ(u_map.at(u_super), 2);
  EXPECT_TRUE(u_map.contains(2));
  EXPECT_EQ(u_map.at(2), 2);

  // reverse updates: node 2 should now point to u_super with count 2
  auto n2_map = sns.get_neighbor_edge_counts(2);
  EXPECT_FALSE(n2_map.contains(v_super));
  ASSERT_TRUE(n2_map.contains(u_super));
  EXPECT_EQ(n2_map.at(u_super), 2);

  // v_super map cleared
  auto v_map = sns.get_neighbor_edge_counts(v_super);
  EXPECT_TRUE(v_map.empty());
}

TEST(SuperNodeSetTest, CorrectMembersSizeContent) {
  const Graph graph = {{1}, {0}, {}, {}};
  SuperNodeSet s(graph);

  s.merge(0, 1);

  const NodeID root = s.get_super_node(0);
  const auto &members = s.get_super_node_members();
  const auto root_members = members.at(root);

  EXPECT_EQ(root_members.size(), 2);
  EXPECT_TRUE(std::ranges::find(root_members.begin(), root_members.end(),0) != root_members.end());
  EXPECT_TRUE(std::ranges::find(root_members.begin(), root_members.end(), 1) != root_members.end());
}

TEST(SuperNodeSetTest, CorrectSizeAfterMerge) {
  const Graph graph = {{1}, {0}, {3}, {2}};
  SuperNodeSet s(graph);
  s.merge(0, 1);
  s.merge(2, 3);

  EXPECT_EQ(s.get_super_nodes().size(), 2);
}

TEST(SuperNodeSetTest, SimpleEdgeCountWithMerge) {
  const Graph graph = {{1}, {0}};
  SuperNodeSet s(graph);
  s.merge(0, 1);

  const NodeID root = s.get_super_node(0);
  EXPECT_EQ(s.get_neighbor_edge_counts(root).at(root), 2);
}

TEST(SuperNodeSetTest, Merge_Basic) {
  const Graph g = {
      {1},    // 0
      {0, 2}, // 1
      {1}     // 2
  };
  SuperNodeSet s(g);

  s.merge(0, 1);

  // Supernode relationship
  EXPECT_EQ(s.get_super_node(0), s.get_super_node(1));

  // Num vertices
  EXPECT_EQ(s.get_dsu().size(0), 2);
  EXPECT_EQ(s.get_dsu().size(1), 2);

  // Same as for UpdateNeighborEdgeCounts_Basic
  auto u_map = s.get_neighbor_edge_counts(0);
  EXPECT_FALSE(u_map.contains(1));
  ASSERT_TRUE(u_map.contains(0));
  EXPECT_EQ(u_map.at(0), 2);
  ASSERT_TRUE(u_map.contains(2));
  EXPECT_EQ(u_map.at(2), 1);
  EXPECT_EQ(u_map.size(), 2);

  auto n2 = s.get_neighbor_edge_counts(2);
  EXPECT_FALSE(n2.contains(1));
  ASSERT_TRUE(n2.contains(s.get_super_node(0)));
  EXPECT_EQ(n2.at(s.get_super_node(0)), 1);

  // v should now refer to u, hence the neighbor edge counts should be the same
  auto v_map = s.get_neighbor_edge_counts(1);
  EXPECT_EQ(u_map, v_map);
}

TEST(SuperNodeSetTest, Merge_WithSharedNeighbor) {
  const Graph g = {
      {1, 2}, // 0
      {0, 2}, // 1
      {0, 1}  // 2
  };
  SuperNodeSet s(g);

  s.merge(0, 1);

  // Supernode relationship
  EXPECT_EQ(s.get_super_node(0), s.get_super_node(1));

  // Num vertices
  EXPECT_EQ(s.get_dsu().size(0), 2);
  EXPECT_EQ(s.get_dsu().size(1), 2);

  // Same as for UpdateNeighborEdgeCounts_WithSharedNbrs
  auto u_map = s.get_neighbor_edge_counts(0);
  EXPECT_FALSE(u_map.contains(1));
  ASSERT_TRUE(u_map.contains(0));
  EXPECT_EQ(u_map.at(0), 2);
  ASSERT_TRUE(u_map.contains(2));
  EXPECT_EQ(u_map.at(2), 2);
  EXPECT_EQ(u_map.size(), 2);

  auto n2 = s.get_neighbor_edge_counts(2);
  EXPECT_FALSE(n2.contains(1));
  ASSERT_TRUE(n2.contains(s.get_super_node(0)));
  EXPECT_EQ(n2.at(s.get_super_node(0)), 2);

  // v should now refer to u, hence the neighbor edge counts should be the same
  auto v_map = s.get_neighbor_edge_counts(1);
  EXPECT_EQ(u_map, v_map);
}

// Calling merge on already-merged nodes should not change anything.
TEST(SuperNodeSetTest, Merge_UnchangedDoubleCall) {
  const Graph g = {
      {1},    // 0
      {0, 2}, // 1
      {1}     // 2
  };
  SuperNodeSet s(g);

  // First merge
  s.merge(0, 1);

  // Supernode relationship
  EXPECT_EQ(s.get_super_node(0), s.get_super_node(1));

  EXPECT_EQ(s.get_dsu().size(0), 2);

  auto u_map_before = s.get_neighbor_edge_counts(0);
  auto n2_before = s.get_neighbor_edge_counts(2);

  // Second merge of the same pair: should be a no-op
  s.merge(0, 1);

  // Supernode relationship
  EXPECT_EQ(s.get_super_node(0), s.get_super_node(1));

  EXPECT_EQ(s.get_dsu().size(0), 2);

  // Maps unchanged
  auto u_map_after = s.get_neighbor_edge_counts(0);
  auto n2_after = s.get_neighbor_edge_counts(2);

  EXPECT_EQ(u_map_after.size(), u_map_before.size());
  for (const auto &[k, v] : u_map_before) {
    ASSERT_TRUE(u_map_after.contains(k));
    EXPECT_EQ(u_map_after.at(k), v);
  }

  EXPECT_EQ(n2_after.size(), n2_before.size());
  for (const auto &[k, v] : n2_before) {
    ASSERT_TRUE(n2_after.contains(k));
    EXPECT_EQ(n2_after.at(k), v);
  }
}

TEST(SuperNodeSetTest, Merge_AllIntoOne) {
  const Graph g = {
      {1},    // 0
      {0, 2}, // 1
      {1, 3}, // 2
      {2}     // 3
  };
  SuperNodeSet s(g);

  s.merge(0, 1);
  s.merge(2, 3);
  s.merge(0, 2); // merges the two components

  // All nodes should resolve to the same supernode
  const auto root = s.get_super_node(0);
  EXPECT_EQ(s.get_super_node(1), root);
  EXPECT_EQ(s.get_super_node(2), root);
  EXPECT_EQ(s.get_super_node(3), root);

  // Size of the merged component
  EXPECT_EQ(s.get_dsu().size(0), 4);

  // Only internal edges remain (no external neighbors)
  auto root_map = s.get_neighbor_edge_counts(root);
  ASSERT_EQ(root_map.size(), 1);
  ASSERT_TRUE(root_map.contains(root));

  // 0-1, 1-2, 2-3 are 3 undirected edges => 6 directed counts
  EXPECT_EQ(root_map.at(root), 6);

  // All non-root indices should equal the root map
  for (int v = 0; v < 4; ++v) {
    if (v == root)
      continue;
    auto m = s.get_neighbor_edge_counts(v);
    EXPECT_EQ(m, root_map);
  }
}

TEST(SuperNodeSetTest, Merge_SelfMerge) {
  const Graph g = {
      {1}, // 0
      {0}  // 1
  };
  SuperNodeSet s(g);

  // Before merge: separate sets
  EXPECT_NE(s.get_super_node(0), s.get_super_node(1));
  EXPECT_EQ(s.get_dsu().size(0), 1);
  EXPECT_EQ(s.get_dsu().size(1), 1);

  // Self-merge should do nothing
  s.merge(0, 0);
  EXPECT_NE(s.get_super_node(0), s.get_super_node(1));
  EXPECT_EQ(s.get_dsu().size(0), 1);
  EXPECT_EQ(s.get_dsu().size(1), 1);

  // Merge(0,1) then self-merge again
  s.merge(0, 1);

  // Supernode relationship
  EXPECT_EQ(s.get_super_node(0), s.get_super_node(1));

  EXPECT_EQ(s.get_dsu().size(0), 2);

  auto u_map_before = s.get_neighbor_edge_counts(0);
  s.merge(0, 0); // still a no-op
  auto u_map_after = s.get_neighbor_edge_counts(0);

  EXPECT_EQ(u_map_after.size(), u_map_before.size());
  for (const auto &[k, v] : u_map_before) {
    ASSERT_TRUE(u_map_after.contains(k));
    EXPECT_EQ(u_map_after.at(k), v);
  }
}