#include "GraphTestUtility.h"
#include "mags/DisjointSetUnion.h"
#include "mags/output.h"

#include <gtest/gtest.h>

namespace mags::out::test {

class OutputTest : public mags::test::GraphTestUtility {};

namespace {
Graph reconstruct_graph(const Representation &rep,
                        const SuperNodeMembers &members, const size_t n) {
  Graph reconstructed(n);

  // 1. Expand Super-edges
  for (const auto &[u_root, v_root] : rep.super_edges) {
    const auto &nodes_u = members.at(u_root);
    const auto &nodes_v = members.at(v_root);

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
} // namespace

TEST_F(OutputTest, BicliqueIdealSuperEdge) {
  const Graph g = triangle;

  Partition p(g.size());
  p.merge(0, 1);
  p.finalize(g);
}

TEST_F(OutputTest, ValidOutput) {
  const Graph g = triangle;

  Partition p(g.size());
  p.merge(0, 1);
  p.finalize(g);

  auto rep = output(g, p);

  // Super-edge (0,1)-(2) should exist because:
  // |Euv| = 2m |Pi_uv| = 2
  // 2 > (2+1)/2
  EXPECT_EQ(rep.super_edges.size(), 1);

  EXPECT_EQ(rep.super_edges[0], std::make_pair(0, 2));

  EXPECT_EQ(rep.minus_corrections.size(), 0);

  EXPECT_EQ(rep.plus_corrections.size(), 1);
  EXPECT_EQ(rep.plus_corrections[0], std::make_pair(0, 1));
}

TEST_F(OutputTest, SparseConnection) {
  const Graph g = {{1}, {0}};

  Partition p(g.size());
  p.merge(0, 1);
  p.finalize(g);

  auto rep = output(g, p);
  EXPECT_EQ(rep.super_edges.size(), 0);
  EXPECT_EQ(rep.plus_corrections.size(), 1);
  EXPECT_EQ(rep.plus_corrections[0], std::make_pair(0, 1));
}

TEST_F(OutputTest, IsLossless) {
  const std::vector graphs = {diamond, triangle, path, isolated,
                              clique}; // Simple graphs only

  for (const auto &g : graphs) {
    Partition p(g.size());
    p.merge(0, 1);
    p.finalize(g);
    auto rep = output(g, p);
    Graph reconstructed = reconstruct_graph(rep, p.get_members(), g.size());
    EXPECT_EQ(g, reconstructed);
  }
}

TEST_F(OutputTest, IdealSuperEdge) {
  // Biclique
  const Graph g = {{2, 3}, {2, 3}, {0, 1}, {0, 1}};
  Partition p(g.size());
  p.merge(0, 1);
  p.merge(2, 3);
  p.finalize(g);

  Representation r = output(g, p);

  EXPECT_EQ(r.super_edges.size(), 1);
  EXPECT_TRUE(r.plus_corrections.empty());
  EXPECT_TRUE(r.minus_corrections.empty());

  // Verify the super-edge connects the correct roots
  NodeID root_u = p.find_super_node(0);
  NodeID root_v = p.find_super_node(2);

  EXPECT_EQ(r.super_edges[0],
            std::make_pair(std::min(root_u, root_v), std::max(root_u, root_v)));
}

TEST_F(OutputTest, BicliqueMinusOne) {
  // 2x2 Biclique missing one edge (0,3)
  Graph g = {{2}, {2, 3}, {0, 1}, {1}};

  Partition p(g.size());
  p.merge(0, 1);
  p.merge(2, 3);
  p.finalize(g);

  Representation r = output(g, p);

  // Should still use a super-edge because 3 edges > threshold 2.5
  EXPECT_EQ(r.super_edges.size(), 1);
  EXPECT_EQ(r.minus_corrections.size(), 1);
  EXPECT_TRUE(r.plus_corrections.empty());

  // The minus correction should be the missing edge (0,3)
  EXPECT_EQ(r.minus_corrections[0], std::make_pair(0, 3));
}

TEST_F(OutputTest, SparsePairPlusCorrection) {
  // Only 1 edge (0,2) exists between the two super-nodes
  Graph g = {{2}, {}, {0}, {}};

  Partition p(g.size());
  p.merge(0, 1);
  p.merge(2, 3);
  p.finalize(g);

  auto rep = output(g, p);

  // Threshold not met (1 < 2.5), so no super-edge
  EXPECT_EQ(rep.super_edges.size(), 0);
  EXPECT_TRUE(rep.minus_corrections.empty());

  // The single edge becomes a plus correction
  EXPECT_EQ(rep.plus_corrections.size(), 1);
  EXPECT_EQ(rep.plus_corrections[0], std::make_pair(0, 2));
}

TEST_F(OutputTest, IdentityWithNoMerges) {
  const Graph g = diamond;
  Partition p(g.size()); // No merge() calls
  p.finalize(g);

  auto rep = output(g, p);

  EXPECT_EQ(rep.super_edges.size(), 0);
  EXPECT_EQ(rep.plus_corrections.size(), g.size());
  EXPECT_EQ(g, reconstruct_graph(rep, p.get_members(), g.size()));
}

TEST_F(OutputTest, InternalEdgesMatchOriginalBehavior) {
  const Graph g = triangle; // 3 nodes, 3 edges (a Clique K3)
  Partition p(g.size());
  p.merge(0, 1);
  p.merge(1, 2); // All merged into one super-node
  p.finalize(g);

  auto rep = output(g, p);

  // Math for Triangle: sz=3, internal_edges=3
  // pi_uu = (3 * 2) / 2 = 3
  // Threshold: 2 * 3 > 3 -> 6 > 3 (True)

  // The original code WILL create a self-loop super-edge here
  EXPECT_EQ(rep.super_edges.size(), 1);
  EXPECT_EQ(rep.super_edges[0].first, rep.super_edges[0].second);

  // It should be a perfect summary, so corrections are empty
  EXPECT_TRUE(rep.plus_corrections.empty());
  EXPECT_TRUE(rep.minus_corrections.empty());

  EXPECT_EQ(g, reconstruct_graph(rep, p.get_members(), g.size()));
}

TEST_F(OutputTest, InternalMinusCorrection) {
  // 4 nodes, but missing one internal edge (0,1)
  // Internal edges: 5. Max possible: 6.
  // 2 * 5 > 6 (True) -> Should be self-loop + 1 minus correction
  Graph g = {{1, 2, 3}, {0, 2, 3}, {0, 1, 3}, {0, 1, 2}};
  // Manually remove edge (0,1)
  std::erase(g[0], 1); std::erase(g[1], 0);

  Partition p(4);
  for(int i=1; i<4; ++i) p.merge(0, i);
  p.finalize(g);

  auto rep = output(g, p);

  EXPECT_EQ(rep.super_edges.size(), 1);
  EXPECT_EQ(rep.super_edges[0].first, rep.super_edges[0].second); // Self-loop
  EXPECT_EQ(rep.minus_corrections.size(), 1);
  EXPECT_EQ(rep.minus_corrections[0], std::make_pair(0, 1));
}

TEST_F(OutputTest, DisconnectedSuperNodes) {
  // Two separate components: {0,1} and {2,3}
  Graph g = {{1}, {0}, {3}, {2}};
  Partition p(4);
  p.merge(0, 1);
  p.merge(2, 3);
  p.finalize(g);

  auto rep = output(g, p);

  // There should be NO super-edge between root(0) and root(2)
  // because e_uv is 0
  EXPECT_EQ(rep.super_edges.size(), 0);
  // Internal edges should still be captured as plus corrections
  EXPECT_EQ(rep.plus_corrections.size(), 2);
}

} // namespace mags::out::test
