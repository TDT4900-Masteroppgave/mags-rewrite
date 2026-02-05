#ifndef MAGS_REWRITE_OUTPUT_H
#define MAGS_REWRITE_OUTPUT_H
#include "SuperNodeSet.h"
#include "candidate_generation.h"

#include <utility>
#include <vector>

#include "types.h"

namespace mags::out {

Representation output(const Graph &graph, const SuperNodeSet &p);

} // namespace mags::out

#endif // MAGS_REWRITE_OUTPUT_H