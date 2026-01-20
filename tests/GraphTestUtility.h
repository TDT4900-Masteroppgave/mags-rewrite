#ifndef MAGS_REWRITE_TEST_UTIL_H
#define MAGS_REWRITE_TEST_UTIL_H
#include "mags/candidate_generation.h"
#include "mags/types.h"
#include <gtest/gtest.h>

namespace mags::test {
class GraphTestUtility : public testing::Test {
protected:
  Graph diamond;
  Graph triangle;
  Graph path;
  Graph isolated;
  Graph star;
  Graph ladder;
  Graph clique;

  void SetUp() override;

  static Graph create_ladder_graph();

  static Graph create_star_graph();

  static Graph create_clique_graph();

  static int signature_matches(int u, int v, const cg::SignatureMatrix &sigs);

  static bool has_pair(const cg::CandidatePairSet &cp, int u, int v);
};
} // namespace mags::test

#endif // MAGS_REWRITE_TEST_UTIL_H
