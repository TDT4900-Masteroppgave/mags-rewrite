#include "util.h"

#include <algorithm>

namespace util {
    void sort_neighbors(mags::Graph& graph) {
        for (auto& neighbors : graph) {
            std::ranges::sort(neighbors);
        }
    }
}

