#ifndef MAGS_REWRITE_DISJOINTSETUNION_H
#define MAGS_REWRITE_DISJOINTSETUNION_H
#include "mags/types.h"

#include <vector>

namespace mags {
class DisjointSetUnion {
  mutable std::vector<int> parents;
  std::vector<int> sizes;

public:
  DisjointSetUnion() = default;
  explicit DisjointSetUnion(int n);

  [[nodiscard]] NodeID find(NodeID node) const;

  void unite(NodeID u, NodeID v);

  [[nodiscard]] int size(NodeID node) const;

#ifdef UNIT_TESTING
  /// @brief Only for internal verification of path compression in tests
  [[nodiscard]] NodeID get_direct_parent(NodeID i) const;
#endif
};
} // namespace mags
#endif // MAGS_REWRITE_DISJOINTSETUNION_H
