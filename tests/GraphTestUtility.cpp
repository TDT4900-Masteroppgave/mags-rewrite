#include "GraphTestUtility.h"
#include "mags/candidate_generation.h"
#include "mags/types.h"
#include "mags/util.h"

#include <gtest/gtest.h>

void GraphTestUtility::SetUp() {
  mags::cg::SEED = 233;

  diamond = {{1, 2}, {0, 3}, {0, 3}, {1, 2}};
  triangle = {{1, 2}, {0, 2}, {0, 1}};
  path = {{1}, {0, 2}, {1, 3}, {2}};
  isolated = {{1}, {0}, {}, {}, {}};
  star = create_star_graph();
  ladder = create_ladder_graph();
  clique = create_clique_graph();

  for (auto *g :
       {&diamond, &triangle, &star, &path, &ladder, &isolated, &clique}) {
    mags::util::sort_neighbors(*g);
  }
}

mags::Graph GraphTestUtility::create_ladder_graph() {
  mags::Graph g;
  g.assign(15, {});

  g.at(0) = {10, 11, 12, 14, 14};
  for (const int nbr : g.at(0))
    g.at(nbr).push_back(0);

  for (int i = 1; i <= 5; ++i) {
    for (int j = 0; j < i; ++j) {
      constexpr int shared_nbr = 10;
      g.at(i).push_back(shared_nbr);
      g.at(shared_nbr).push_back(i);
    }
  }
  return g;
}

mags::Graph GraphTestUtility::create_star_graph() {
  mags::Graph g(11);
  for (int i = 0; i <= 10; ++i) {
    g.at(0).push_back(i);
    g.at(i).push_back(0);
  }
  return g;
}

mags::Graph GraphTestUtility::create_clique_graph() {
  mags::Graph g(4);
  for (int i = 0; i < g.size(); ++i) {
    for (int j = i + 1; j < g.size(); ++j) {
      g.at(i).push_back(j);
      g.at(j).push_back(i);
    }
  }
  return g;
}

int GraphTestUtility::SignatureMatches(const int u, const int v,
                                       const mags::cg::SignatureMatrix &sigs) {
  int matches = 0;
  for (int h = 0; h < mags::cg::H_FUNCS; ++h) {
    if (sigs.at(u).at(h) == sigs.at(v).at(h))
      matches++;
  }
  return matches;
}

bool GraphTestUtility::HasPair(const mags::cg::CandidatePairSet &cp, int u,
                               int v) {
  const std::pair<mags::NodeID, mags::NodeID> target =
      u < v ? std::make_pair(u, v) : std::make_pair(v, u);

  return cp.contains(target);
}
