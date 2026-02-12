#include "GraphTestUtility.h"
#include "mags/greedy_merge.h"
#include <chrono>

#include <mags/graph_summarize.h>
#include <gtest/gtest.h>

class GraphSummarizeTest : public test::GraphTestUtility {};

TEST_F(GraphSummarizeTest, ValidGraph) {
  write_tmp_file("0 1\n0 2\n2 1");

  const auto r = summarize_from_file(tmp_file_name);

  EXPECT_EQ(r.get_total_cost(), 1);
  EXPECT_EQ(r.super_edges.size(), 1);
  EXPECT_TRUE(r.plus_corrections.empty());
  EXPECT_TRUE(r.minus_corrections.empty());
}

TEST_F(GraphSummarizeTest, StarGraphCompression) {
  // Center: 0, Leaves: 1-5
  // Ideally, {1,2,3,4,5} become a SuperNode, connected to {0}.
  write_tmp_file(
      "0 1\n0 2\n0 3\n0 4\n0 5"
  );

  // t=50, k=40 (Standard defaults)
  const auto r = summarize_from_file(tmp_file_name, 50, 40);

  // Original Edges: 5
  // Ideal Cost: 1 SuperEdge ({0}-{Leaves}) + 0 Corrections = 1
  // Acceptable Cost: < 5
  EXPECT_LT(r.get_total_cost(), 5);
  EXPECT_LT(r.get_relative_size(), 1.0); // Must be compressed

  // In a star graph, we expect 0 corrections if optimized perfectly
  if (r.super_edges.size() > 0) {
      EXPECT_TRUE(r.plus_corrections.empty());
      EXPECT_TRUE(r.minus_corrections.empty());
  }
}

TEST_F(GraphSummarizeTest, CliqueWithMissingEdge) {
  // Nodes 0,1,2,3 fully connected (K4) EXCEPT edge (0,1)
  // Edges: (0,2), (0,3), (1,2), (1,3), (2,3)
  write_tmp_file(
      "0 2\n0 3\n"
      "1 2\n1 3\n"
      "2 3"
  );

  const auto r = summarize_from_file(tmp_file_name, 50, 40);

  // Ideal: 1 SuperNode {0,1,2,3} with Self-Loop.
  // Errors: (0,1) is missing, so 1 Minus Correction.
  // Total Cost: 1 SE + 1 MC = 2.
  // Original Edges: 5.
  EXPECT_LT(r.get_total_cost(), 5);
  EXPECT_NE(r.plus_corrections.size(), 5);
}

TEST_F(GraphSummarizeTest, DisconnectedGraph) {
  // Component A: 0-1
  // Component B: 2-3
  write_tmp_file("0 1\n2 3");

  const auto r = summarize_from_file(tmp_file_name, 50, 40);

  // Total edges: 2.
  // Summary should cost exactly 2 (either 2 SuperEdges or 2 Plus Corrections).
  EXPECT_EQ(r.get_total_cost(), 2);
}

TEST_F(GraphSummarizeTest, HandlesDuplicatesAndSelfLoops) {
  // Edge 0-1 repeated 3 times
  // Self-loop 1-1 (should be removed)
  write_tmp_file(
      "0 1\n"
      "0 1\n"
      "1 0\n" // Reversed duplicate
      "1 1"   // Self loop
  );

  const auto r = summarize_from_file(tmp_file_name, 50, 40);

  // Should result in exactly 1 valid edge (0-1)
  EXPECT_EQ(r.get_total_cost(), 1);
  // Original graph had effectively 1 edge, relative size should count based on cleaned or original?
  // Usually relative size is based on Cleaned Graph size in MAGS.
}

TEST_F(GraphSummarizeTest, EmptyFile) {
  write_tmp_file("");

  // Should not throw exception
  EXPECT_NO_THROW({
      const auto r = summarize_from_file(tmp_file_name, 50, 40);
      EXPECT_EQ(r.get_total_cost(), 0);
  });
}