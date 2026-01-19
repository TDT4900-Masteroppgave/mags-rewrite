#include "mags/util.h"

#include <algorithm>

namespace mags::util {
    void sort_neighbors(Graph& graph) {
        for (auto& neighbors : graph) {
            std::ranges::sort(neighbors);
        }
    }
}

