#include "mags/mags.h"

#include "mags/greedy_merge.h"

namespace mags {
out::Representation mags(const Graph &graph, const int t, const int k) {
  cg::CandidatePairSet cp = cg::generate_candidates(graph, k);
  const SuperNodeSet sp = gm::greedy_merge(graph, t, cp);
  return out::output(graph, sp);
}
}

