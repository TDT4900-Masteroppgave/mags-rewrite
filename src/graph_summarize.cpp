#include "mags/graph_summarize.h"

#include <mags/file_util.h>
#include <mags/mags.h>
#include <mags/preprocess.h>

namespace mags {

Representation summarize_from_file(const std::string &path, const int t,
                                    const int k) {
  Graph inputGraph = io::read_from_file(path);
  const Graph clean = preprocess::clean_graph(inputGraph);
  const Representation r = mags::mags(clean, t, k);

  return r;
}

} // namespace mags
