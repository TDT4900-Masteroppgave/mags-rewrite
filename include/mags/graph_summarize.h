#ifndef MAGS_REWRITE_GRAPH_SUMMARIZE_H
#define MAGS_REWRITE_GRAPH_SUMMARIZE_H
#include <mags/output.h>

namespace mags {
Representation summarize_from_file(const std::string &path, int t = 50,
                                   int k = 40);
}

#endif // MAGS_REWRITE_GRAPH_SUMMARIZE_H
