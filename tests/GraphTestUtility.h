#ifndef MAGS_REWRITE_TEST_UTIL_H
#define MAGS_REWRITE_TEST_UTIL_H
#include "mags/candidate_generation.h"
#include "mags/types.h"
#include <gtest/gtest.h>

class GraphTestUtility : public testing::Test {
protected:
  mags::Graph diamond;
  mags::Graph triangle;
  mags::Graph path;
  mags::Graph isolated;
  mags::Graph star;
  mags::Graph ladder;
  mags::Graph clique;

  void SetUp() override;

  static mags::Graph create_ladder_graph();

  static mags::Graph create_star_graph();

  static mags::Graph create_clique_graph();

  static int SignatureMatches(int u, int v,
                              const mags::cg::SignatureMatrix &sigs);

  static bool HasPair(const mags::cg::CandidatePairSet &cp, int u, int v);
};

#endif // MAGS_REWRITE_TEST_UTIL_H
