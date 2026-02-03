#include "GraphTestUtility.h"
#include "mags/preprocess.h"

#include <gtest/gtest.h>

namespace mags::preprocess::test {
class PreprocessTest : public mags::test::GraphTestUtility {};

TEST_F(PreprocessTest, SelfLoopRemoval) {
  Graph g = {{0, 1}, {0}}; // Node 0 contains self loop
  const Graph clean_g = clean_graph(g);
  EXPECT_EQ(clean_g, Graph({{1}, {0}})); // removes self loop
}

TEST_F(PreprocessTest, DuplicateEdgeRemoval) {
  Graph g = {{1, 1}, {0, 0}}; // Contains duplicates
  const Graph clean_g = clean_graph(g);
  EXPECT_EQ(clean_g, Graph({{1}, {0}})); // removes duplicates
}

TEST_F(PreprocessTest, SymmetryEnforcement) {
  // Node 1 is missing connection to node 0
  Graph g = {{1, 2}, {2}, {0, 1}};
  const Graph clean_g = clean_graph(g);
  EXPECT_EQ(clean_g, Graph({{1, 2}, {0, 2}, {0, 1}}));
}

TEST_F(PreprocessTest, IsolatedNodeHandling) {
  Graph g = {{1}, {}};
  const Graph clean_g = clean_graph(g);
  EXPECT_EQ(clean_g, Graph({{1}, {0}}));
}

TEST_F(PreprocessTest, NeighboringOrdering) {
  Graph g = {{2, 1}, {0}, {0}};
  const Graph clean_g = clean_graph(g);
  EXPECT_EQ(clean_g, Graph({{1, 2}, {0}, {0}}));
}

TEST_F(PreprocessTest, EmptyGraph) {
  Graph g = {};
  const Graph clean_g = clean_graph(g);
  EXPECT_EQ(clean_g.size(), 0);
}

TEST_F(PreprocessTest, SmallestGraph) {
  Graph g = {{1}, {0}};
  const Graph clean_g = clean_graph(g);
  EXPECT_EQ(clean_g.size(), 2);
}

}

