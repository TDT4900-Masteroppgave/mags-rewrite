#ifndef MAGS_REWRITE_MAGS_H
#define MAGS_REWRITE_MAGS_H
#include "output.h"
#include "types.h"

namespace mags {

Representation mags(const Graph &graph, int t = 50, int k = 40);

} // namespace mags

#endif // MAGS_REWRITE_MAGS_H
