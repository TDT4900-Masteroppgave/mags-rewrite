#ifndef MAGS_REWRITE_TEST_UTIL_H
#define MAGS_REWRITE_TEST_UTIL_H
#include "mags/candidate_generation.h"
#include "mags/types.h"
#include <gtest/gtest.h>

namespace mags::out {
struct Representation;
}
namespace mags::test {
class GraphTestUtility : public testing::Test {
protected:
  Graph diamond;
  Graph triangle;
  Graph path;
  Graph isolated;
  Graph star;
  Graph ladder;
  Graph clique;
  std::string tmp_file_name;

  void SetUp() override;

  static Graph create_ladder_graph();

  static Graph create_star_graph();

  static Graph create_clique_graph();

  static Graph reconstruct_graph(const out::Representation &rep, size_t n);

  static size_t get_edge_count(const Graph &graph);

  void write_tmp_file(const std::string &content) const;
};
} // namespace mags::test

#endif // MAGS_REWRITE_TEST_UTIL_H
