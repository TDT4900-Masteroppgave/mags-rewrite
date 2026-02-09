#include "mags/mags.h"
#include "mags/greedy_merge.h"
#include "mags/util_time.h"

namespace mags {
out::Representation mags(const Graph &graph, const int t, const int k) {
  
  cg::CandidatePairSet cp = cg::generate_candidates(graph, k);
  const SuperNodeSet sp = gm::greedy_merge(graph, t, cp);
  timing::set_time(timing::merge_time);
  
  out::Representation representation = out::output(graph, sp);
  timing::set_time(timing::encoding_time);

  return representation;
}
}

