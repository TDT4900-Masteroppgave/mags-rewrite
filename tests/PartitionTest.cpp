#include <gtest/gtest.h>
#include <mags/Partition.h>
#include <mags/candidate_generation.h>

using namespace mags;

TEST(PartitionTest, Initialization) {
  constexpr size_t n = 5;
  Partition p(n);
  const Graph empty_graph{n, std::vector<NodeID>{}};

  p.finalize(empty_graph);
  EXPECT_EQ(p.get_super_nodes().size(), n);
}

TEST(PartitionTest, MergePersistence) {
  constexpr size_t n = 5;
  Partition p(n);

  p.merge(0, 1);
  EXPECT_EQ(p.find_super_node(0), p.find_super_node(1));
}

TEST(PartitionTest, EmpptyFinalize) {
  constexpr size_t n = 5;
  Partition p(n);
  const Graph empty_graph{n, std::vector<NodeID>{}};

  EXPECT_NO_THROW(p.finalize(empty_graph));
  for (NodeID root : p.get_super_nodes()) {
    EXPECT_TRUE(p.get_edge_counts().at(root).empty());
  }
}

TEST(PartitionTest, CorrectSizeAfterMerge) {
  const Graph g = {{1}, {0}, {3}, {2}};
  Partition p(g.size());
  p.merge(0, 1);
  p.merge(2, 3);
  p.finalize(g);

  EXPECT_EQ(p.get_super_nodes().size(), 2);
}

TEST(PartitonTest, CorrectMembersSizeContent) {
  const Graph g = {{1}, {0}, {}, {}};
  Partition p(g.size());

  p.merge(0, 1);
  p.finalize(g);

  const NodeID root = p.find_super_node(0);
  const auto &members = p.get_members().at(root);

  EXPECT_EQ(members.size(), 2);
  EXPECT_TRUE(std::ranges::find(members.begin(), members.end(), 0) !=
              members.end());
  EXPECT_TRUE(std::ranges::find(members.begin(), members.end(), 1) !=
              members.end());
}

TEST(PartitonTest, SimpleEdgeCount) {
  const Graph g = {{1}, {0}};
  Partition p(g.size());

  p.finalize(g);
  const NodeID r0 = p.find_super_node(0);
  const NodeID r1 = p.find_super_node(1);
  EXPECT_EQ(p.get_edge_counts().at(r0).at(r1), 1);
}

TEST(PartitionTest, SimpleEdgeCountWithMerge) {
  const Graph g = {{1}, {0}};
  Partition p(g.size());
  p.merge(0, 1);
  p.finalize(g);

  const NodeID root = p.find_super_node(0);
  EXPECT_EQ(p.get_edge_counts().at(root).at(root), 1);
}

TEST(PartitionTest, SimpleCatesianProduct) {
  const Graph g = {{1}, {0}, {3}, {2}};
  Partition p(g.size());
  p.merge(0, 1); // Super-Node A size 2
  p.merge(2, 3); // Super-Node B size 2
  p.finalize(g);

  // |Pi_uv| = 2 * 2 = 4
  EXPECT_EQ(p.get_cartesian_product(0, 2), 4);
}

TEST(PartitionTest, CartesianPrdouctSelfLoop) {
  const Graph g = {{1}, {0}, {2}};
  Partition p(g.size());
  p.merge(0, 1);
  p.merge(1, 2); // Super-Node size 3
  p.finalize(g);

  EXPECT_EQ(p.get_cartesian_product(0, 0), 3);
}

TEST(PartitionTest, CartesianProductSingleNode) {
  const Graph g = {{0}};
  Partition p(g.size());
  p.finalize(g);

  EXPECT_EQ(p.get_cartesian_product(0, 0), 0);
}