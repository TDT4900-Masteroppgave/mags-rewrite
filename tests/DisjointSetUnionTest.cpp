#include "mags/DisjointSetUnion.h"
#include <gtest/gtest.h>
#include "GraphTestUtility.h"

using namespace mags;

class DisjointSetUnionTest : public testing::Test {
protected:
  constexpr static int n = 10;
  DisjointSetUnion dsu;
  void SetUp() override { dsu = DisjointSetUnion(n); }
};

TEST_F(DisjointSetUnionTest, Inititalize) {
  for (int i = 0; i < n; ++i) {
    EXPECT_EQ(dsu.find(i), i);
    EXPECT_EQ(dsu.size(i), 1);
  }
}

TEST_F(DisjointSetUnionTest, SimpleUnite) {
  dsu.unite(0, 1);
  EXPECT_EQ(dsu.find(0), dsu.find(1));
  EXPECT_EQ(dsu.size(0), dsu.size(1));
}

TEST_F(DisjointSetUnionTest, TransitiveUnion) {
  dsu.unite(0, 1);
  dsu.unite(1, 2);
  EXPECT_EQ(dsu.find(0), dsu.find(2));
}

TEST_F(DisjointSetUnionTest, UnionBySize) {
  dsu.unite(1, 2);                          // Group with size 2
  const NodeID expected_root = dsu.find(1); // Root before unite
  dsu.unite(0, 1); // Unite group 2 of size 2 with a group of size 1

  EXPECT_EQ(dsu.find(0), expected_root);
  EXPECT_EQ(dsu.size(0), 3);
}

TEST_F(DisjointSetUnionTest, PathCompression) {
  dsu.unite(0, 1); // Group A
  dsu.unite(2, 3); // Group B
  dsu.unite(0, 2); // Unite roots of both groups

  const NodeID root = dsu.find(0); // capture root node before unite
  EXPECT_EQ(dsu.find(3), root);  // Should flatten the graph

  // Every node should point directly to the root
  EXPECT_EQ(dsu.get_direct_parent(0), root);
  EXPECT_EQ(dsu.get_direct_parent(1), root);
  EXPECT_EQ(dsu.get_direct_parent(2), root);
  EXPECT_EQ(dsu.get_direct_parent(3), root);
}

TEST_F(DisjointSetUnionTest, SelfUnion) {
  dsu.unite(0, 0);
  EXPECT_EQ(dsu.find(0), 0);
  EXPECT_EQ(dsu.size(0), 1);
}

TEST_F(DisjointSetUnionTest, FullUnite) {
  for (int i = 0; i < n - 1; ++i) {
    dsu.unite(i, i + 1);
  }

  for (int i = 0; i < n; ++i) {
    EXPECT_EQ(dsu.size(i), n);
  }
}
