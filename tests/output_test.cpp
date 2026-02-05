#include "GraphTestUtility.h"
#include "mags/DisjointSetUnion.h"
#include "mags/output.h"

#include <gtest/gtest.h>

namespace mags::out::test {

class OutputTest : public mags::test::GraphTestUtility {};

TEST_F(OutputTest, ValidOutput) {
  const Graph graph = {{1, 2}, {0, 2}, {0, 1}};

  SuperNodeSet super_nodes(graph);
  super_nodes.merge(0, 1);

  auto rep = output(graph, super_nodes);

  // Super-edge (0,1)-(2) should exist because:
  // |Euv| = 2m |Pi_uv| = 2
  // 2 > (2+1)/2
  EXPECT_EQ(rep.super_edges.size(), 1);

  EXPECT_EQ(rep.super_edges.at(0), std::make_pair(0, 2));

  EXPECT_EQ(rep.minus_corrections.size(), 0);

  EXPECT_EQ(rep.plus_corrections.size(), 1);
  EXPECT_EQ(rep.plus_corrections.at(0), std::make_pair(0, 1));
}

TEST_F(OutputTest, SparseConnection) {
  const Graph graph = {{1}, {0}};

  SuperNodeSet super_nodes(graph);
  super_nodes.merge(0, 1);
  auto rep = output(graph, super_nodes);
  EXPECT_EQ(rep.super_edges.size(), 0);
  EXPECT_EQ(rep.plus_corrections.size(), 1);
  EXPECT_EQ(rep.plus_corrections[0], std::make_pair(0, 1));
}

TEST_F(OutputTest, IsLossless) {
  const std::vector graphs = {diamond, triangle, path, isolated,
                              clique}; // Simple graphs only

  for (const auto &graph : graphs) {
    SuperNodeSet super_nodes(graph);
    super_nodes.merge(0, 1);

    auto rep = output(graph, super_nodes);
    Graph reconstructed = reconstruct_graph(rep, graph.size());
    EXPECT_EQ(graph, reconstructed);
  }
}

TEST_F(OutputTest, IdealSuperEdge) {
  // Biclique
  const Graph graph = {{2, 3}, {2, 3}, {0, 1}, {0, 1}};
  SuperNodeSet super_nodes(graph);
  super_nodes.merge(0, 1);
  super_nodes.merge(2, 3);

  Representation r = output(graph, super_nodes);

  EXPECT_EQ(r.super_edges.size(), 1);
  EXPECT_TRUE(r.plus_corrections.empty());
  EXPECT_TRUE(r.minus_corrections.empty());

  // Verify the super-edge connects the correct roots
  NodeID root_u = super_nodes.get_super_node(0);
  NodeID root_v = super_nodes.get_super_node(2);

  EXPECT_EQ(r.super_edges[0],
            std::make_pair(std::min(root_u, root_v), std::max(root_u, root_v)));
}

TEST_F(OutputTest, BicliqueMinusOne) {
  // 2x2 Biclique missing one edge (0,3)
  Graph graph = {{2}, {2, 3}, {0, 1}, {1}};

  SuperNodeSet super_nodes(graph);
  super_nodes.merge(0, 1);
  super_nodes.merge(2, 3);

  Representation r = output(graph, super_nodes);

  // Should still use a super-edge because 3 edges > threshold 2.5
  EXPECT_EQ(r.super_edges.size(), 1);
  EXPECT_EQ(r.minus_corrections.size(), 1);
  EXPECT_TRUE(r.plus_corrections.empty());

  // The minus correction should be the missing edge (0,3)
  EXPECT_EQ(r.minus_corrections[0], std::make_pair(0, 3));
}

TEST_F(OutputTest, SparsePairPlusCorrection) {
  // Only 1 edge (0,2) exists between the two super-nodes
  Graph graph = {{2}, {}, {0}, {}};

  SuperNodeSet super_nodes(graph);
  super_nodes.merge(0, 1);
  super_nodes.merge(2, 3);

  auto rep = output(graph, super_nodes);

  // Threshold not met (1 < 2.5), so no super-edge
  EXPECT_EQ(rep.super_edges.size(), 0);
  EXPECT_TRUE(rep.minus_corrections.empty());

  // The single edge becomes a plus correction
  EXPECT_EQ(rep.plus_corrections.size(), 1);
  EXPECT_EQ(rep.plus_corrections[0], std::make_pair(0, 2));
}

TEST_F(OutputTest, IdentityWithNoMerges) {
  const Graph graph = diamond;
  SuperNodeSet super_nodes(graph); // No merge() calls

  auto rep = output(graph, super_nodes);

  EXPECT_EQ(rep.super_edges.size(), 0);
  EXPECT_EQ(rep.plus_corrections.size(), graph.size());
  EXPECT_EQ(graph, reconstruct_graph(rep, graph.size()));
}

TEST_F(OutputTest, InternalEdgesMatchOriginalBehavior) {
  const Graph graph = triangle; // 3 nodes, 3 edges (a Clique K3)
  SuperNodeSet super_nodes(graph);
  super_nodes.merge(0, 1);
  super_nodes.merge(1, 2); // All merged into one super-node

  auto rep = output(graph, super_nodes);

  // Math for Triangle: sz=3, internal_edges=3
  // pi_uu = (3 * 2) / 2 = 3
  // Threshold: 2 * 3 > 3 -> 6 > 3 (True)

  // The original code WILL create a self-loop super-edge here
  EXPECT_EQ(rep.super_edges.size(), 1);
  EXPECT_EQ(rep.super_edges[0].first, rep.super_edges[0].second);

  // It should be a perfect summary, so corrections are empty
  EXPECT_TRUE(rep.plus_corrections.empty());
  EXPECT_TRUE(rep.minus_corrections.empty());

  EXPECT_EQ(graph, reconstruct_graph(rep, graph.size()));
}

TEST_F(OutputTest, InternalMinusCorrection) {
  // 4 nodes, but missing one internal edge (0,1)
  // Internal edges: 5. Max possible: 6.
  // 2 * 5 > 6 (True) -> Should be self-loop + 1 minus correction
  Graph graph = {{1, 2, 3}, {0, 2, 3}, {0, 1, 3}, {0, 1, 2}};
  // Manually remove edge (0,1)
  std::erase(graph[0], 1);
  std::erase(graph[1], 0);

  SuperNodeSet super_nodes(graph);
  for (int i = 1; i < 4; ++i)
    super_nodes.merge(0, i);

  auto rep = output(graph, super_nodes);

  EXPECT_EQ(rep.super_edges.size(), 1);
  EXPECT_EQ(rep.super_edges[0].first, rep.super_edges[0].second); // Self-loop
  EXPECT_EQ(rep.minus_corrections.size(), 1);
  EXPECT_EQ(rep.minus_corrections[0], std::make_pair(0, 1));
}

TEST_F(OutputTest, DisconnectedSuperNodes) {
  // Two separate components: {0,1} and {2,3}
  Graph graph = {{1}, {0}, {3}, {2}};
  SuperNodeSet super_nodes(graph);
  super_nodes.merge(0, 1);
  super_nodes.merge(2, 3);

  auto rep = output(graph, super_nodes);

  // There should be NO super-edge between root(0) and root(2)
  // because e_uv is 0
  EXPECT_EQ(rep.super_edges.size(), 0);
  // Internal edges should still be captured as plus corrections
  EXPECT_EQ(rep.plus_corrections.size(), 2);
}

} // namespace mags::out::test
