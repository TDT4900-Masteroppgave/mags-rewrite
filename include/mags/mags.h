#ifndef MAGS_REWRITE_MAGS_H
#define MAGS_REWRITE_MAGS_H
#include "file_util.h"
#include "output.h"
#include "preprocess.h"
#include "types.h"

namespace mags {

out::Representation mags(const Graph &graph, int t = 50, int k = 30);

inline out::Representation summarize_graph(const std::string &path, const int t,
                                           const int k) {
  Graph graph = io::read_from_file(path);
  const Graph clean_graph = preprocess::clean_graph(graph);

  return mags(clean_graph, t, k);
}

} // namespace mags

#endif // MAGS_REWRITE_MAGS_H
