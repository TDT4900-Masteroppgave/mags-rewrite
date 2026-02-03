#include "mags/DisjointSetUnion.h"

#include <numeric>

namespace mags {

DisjointSetUnion::DisjointSetUnion(const size_t n) : parents(n), sizes(n, 1) {
  std::iota(parents.begin(), parents.end(), 0);
}

[[nodiscard]] NodeID DisjointSetUnion::find(const NodeID node) const {
  if (parents[node] != node)
    parents[node] = find(parents[node]);

  return parents[node];
}

void DisjointSetUnion::unite(const NodeID u, const NodeID v) {
  NodeID parent_u = find(u);
  NodeID parent_v = find(v);

  if (parent_u == parent_v)
    return;

  if (sizes[parent_u] < sizes[parent_v])
    std::swap(parent_u, parent_v);

  parents[parent_v] = parent_u;
  sizes[parent_u] += sizes[parent_v];
}

[[nodiscard]] int DisjointSetUnion::size(const NodeID node) const {
  return sizes[find(node)];
}

#ifdef UNIT_TESTING
[[nodiscard]] NodeID DisjointSetUnion::get_direct_parent(const NodeID i) const {
  return parents[i];
}
#endif

} // namespace mags
