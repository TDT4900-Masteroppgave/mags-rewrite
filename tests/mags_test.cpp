#include "GraphTestUtility.h"
#include "mags/file_util.h"
#include "mags/mags.h"
#include "mags/output.h"
#include "mags/preprocess.h"

#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>

namespace mags::test {
class MagsTest : public GraphTestUtility {
protected:
  static void VerifyReconstruction(const Graph &original,
                            const Representation &rep) {
    const Graph reconstructed = reconstruct_graph(rep, original.size());
    EXPECT_EQ(original, reconstructed)
        << "Reconstructed graph does not match original!";
  }
};

TEST_F(MagsTest, CorrectRepresentationCost) {
  Graph original = {{0, 1}, {0, 2}, {1, 2}, {2, 3}, {3, 4}};
  const Graph clean = preprocess::clean_graph(original);

  const Representation r = mags::mags(clean);

  // R < |E| + C
  EXPECT_NE(r.get_total_cost(), 0);
  EXPECT_LE(r.get_total_cost(), get_edge_count(clean));
  VerifyReconstruction(clean, r);
}

TEST_F(MagsTest, Clique) {
  Graph original = clique;
  const Graph clean = preprocess::clean_graph(original);

  const Representation r = mags::mags(clean);
  EXPECT_EQ(r.get_total_cost(), 1);
  EXPECT_EQ(r.super_edges.size(), 1);
  EXPECT_EQ(r.plus_corrections.size(), 0);
  EXPECT_EQ(r.minus_corrections.size(), 0);
  VerifyReconstruction(clean, r);
}

TEST_F(MagsTest, Star) {
  Graph original = star;
  const Graph clean = preprocess::clean_graph(original);
  const Representation r = mags::mags(clean);
  EXPECT_NE(r.get_total_cost(), 0);
  EXPECT_LE(r.get_total_cost(), get_edge_count(clean));
  VerifyReconstruction(clean, r);
}

TEST_F(MagsTest, IdenticalNeighbors) {
  Graph original = {{2, 3}, {2, 3}, {3}, {2}};
  const Graph clean = preprocess::clean_graph(original);
  const Representation r = mags::mags(clean);
  EXPECT_EQ(r.get_total_cost(), 2);
  VerifyReconstruction(clean, r);
}

TEST_F(MagsTest, ZeroK) {
  Graph original = path;
  const Graph clean = preprocess::clean_graph(original);
  const Representation r = mags::mags(clean, 50, 0);
  EXPECT_EQ(r.get_total_cost(), get_edge_count(clean));
  VerifyReconstruction(clean, r);
}

TEST_F(MagsTest, HigherThresholdCompactness) {
  Graph original = path;
  const Graph clean = preprocess::clean_graph(original);

  Representation r = mags::mags(clean, 50);
  const size_t cost1 = r.get_total_cost();
  VerifyReconstruction(clean, r);

  r = mags::mags(clean, 5);
  const size_t cost2 = r.get_total_cost();
  EXPECT_LE(cost1, cost2);
  VerifyReconstruction(clean, r);
}

} // namespace mags::test
