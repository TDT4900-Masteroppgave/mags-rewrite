#include "mags/file_util.h"
#include "mags/types.h"

#include <fstream>
#include <filesystem>
#include <gtest/gtest.h>

namespace mags::io::test {
class FileUtilTest : public testing::Test {
protected:
  std::string tmp_file_name = "test_graph.txt";

  void write_tmp_file(const std::string &content) const {
    std::ofstream outfile(tmp_file_name);
    outfile << content;
    outfile.close();
  }

  void SetUp() override { std::filesystem::remove(tmp_file_name); }

  void TearDown() override { std::filesystem::remove(tmp_file_name); }
};

TEST_F(FileUtilTest, StandardEdgeList) {
  write_tmp_file("0 1\n0 2\n2 1");

  const Graph g = read_from_file(tmp_file_name);
  EXPECT_EQ(g.size(), 3);
}

TEST_F(FileUtilTest, InitializeCorrectGraphSize) {
  write_tmp_file("0 100");

  const Graph g = read_from_file(tmp_file_name);
  EXPECT_EQ(g.size(), 101);
}

TEST_F(FileUtilTest, MultipleNeighbors) {
  write_tmp_file("0 1\n0 2\n0 3");
  const Graph g = read_from_file(tmp_file_name);

  // Node 0 should have three neighbors
  ASSERT_EQ(g.size(), 4); // Nodes 0, 1, 2, 3
  EXPECT_EQ(g.at(0).size(), 3);
  EXPECT_EQ(g.at(0), std::vector({1, 2, 3}));
}

TEST_F(FileUtilTest, FileNotFound) {
  const Graph g = read_from_file("file_not_found");
  EXPECT_EQ(g.size(), 0);
}

TEST_F(FileUtilTest, EmpltyFile) {
  write_tmp_file("");
  Graph g = read_from_file(tmp_file_name);
  EXPECT_EQ(g.size(), 0);

  write_tmp_file(" ");
  g = read_from_file(tmp_file_name);
  EXPECT_EQ(g.size(), 0);
}

// Test cases where the second half of an edge is missing
TEST_F(FileUtilTest, PartialEdgeDefinition) {
  write_tmp_file("0");
  const Graph g = read_from_file(tmp_file_name);
  EXPECT_EQ(g.size(), 0);
}

// Test cases with unexpected whitespace or formatting
TEST_F(FileUtilTest, LeadingOrTrailingWhitespace) {
  write_tmp_file(" 0");
  Graph g = read_from_file(tmp_file_name);
  EXPECT_EQ(g.size(), 0);

  write_tmp_file("0\n 1");
  g = read_from_file(tmp_file_name);
  EXPECT_EQ(g.size(), 2);
}

// Test cases with non-numeric data
TEST_F(FileUtilTest, NonNumericCharacters) {
  write_tmp_file("A");
  const Graph g = read_from_file(tmp_file_name);
  EXPECT_EQ(g.size(), 0);
}

// Test cases with multiple newlines or empty segments
TEST_F(FileUtilTest, ExtraNewlines) {
  write_tmp_file("0\n\n1");
  const Graph g = read_from_file(tmp_file_name);
  EXPECT_EQ(g.size(), 2);
}

TEST_F(FileUtilTest, LargeNodeID) {
  write_tmp_file("0\n10000");
  const Graph g = read_from_file(tmp_file_name);
  EXPECT_EQ(g.size(), 10001);
}

} // namespace mags::io::test
