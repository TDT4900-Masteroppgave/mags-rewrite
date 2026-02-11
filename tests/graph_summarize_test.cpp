#include "GraphTestUtility.h"
#include "mags/greedy_merge.h"
#include <chrono>

#include <mags/graph_summarize.h>
#include <gtest/gtest.h>

class GraphSummarizeTest : public test::GraphTestUtility {};

std::string email_data = "../../tests/data/email-Eu-core.txt";

TEST_F(GraphSummarizeTest, ValidGraph) {
  write_tmp_file("0 1\n0 2\n2 1");

  const auto r = summarize_from_file(tmp_file_name);

  EXPECT_EQ(r.get_total_cost(), 1);
  EXPECT_EQ(r.super_edges.size(), 1);
  EXPECT_TRUE(r.plus_corrections.empty());
  EXPECT_TRUE(r.minus_corrections.empty());
}