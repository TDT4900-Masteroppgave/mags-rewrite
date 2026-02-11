#include "mags/graph_summarize.h"

#include <mags/util_file.h>
#include <mags/mags.h>
#include <mags/preprocess.h>
#include "mags/util_time.h"


namespace mags {
                                               
Representation summarize_from_file(const std::string &path, const int t,
const int k) {
  timing::set_time(timing::start_time);
  Graph inputGraph = io::read_from_file(path);
  timing::set_time(timing::read_time);
  const Graph clean = preprocess::clean_graph(inputGraph);
  return mags(clean, t, k);
}

} // namespace mags
