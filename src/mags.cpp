#include "mags/mags.h"

#include "mags/greedy_merge.h"
#include <mags/candidate_generation.h>
#include <mags/output.h>

namespace mags {
Representation mags(const Graph &graph, const int t, const int k) {
  cg::CandidatePairSet cp = cg::generate_candidates(graph, k);
  const SuperNodeSet sp = gm::greedy_merge(graph, t, cp);
  return out::output(graph, sp);
}
}

