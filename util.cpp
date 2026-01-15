#include "util.h"

#include <algorithm>

void sort_neighbors(mags::Graph& graph) {
    for (auto& neighbors : graph) {
        std::ranges::sort(neighbors);
    }
}
