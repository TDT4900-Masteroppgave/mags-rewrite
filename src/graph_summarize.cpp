#include "graph_summarize.h"

#include <mags/file_util.h>
#include <mags/mags.h>
#include <mags/preprocess.h>

out::Representation graph_summarize(const std::string &path, const int t,
                                           const int k) {
  Graph inputGraph = io::read_from_file(path);
  const Graph clean = preprocess::clean_graph(inputGraph);
  const out::Representation r = mags::mags(clean, t, k);

  return r;
}