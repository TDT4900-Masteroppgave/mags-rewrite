#include "GraphTestUtility.h"
#include "mags/greedy_merge.h"
#include "mags/preprocess.h"

#include <chrono>
#include <iomanip>
#include <iostream>

#include <../include/mags/graph_summarize.h>
#include <gtest/gtest.h>

class GraphSummarizeTest : public test::GraphTestUtility {};

std::string email_data = "../../tests/data/email-Eu-core.txt";

TEST_F(GraphSummarizeTest, ValidGraph) {
  write_tmp_file("0 1\n0 2\n2 1");

  const auto r = summarize_from_file(tmp_file_name);

  EXPECT_EQ(r.get_total_cost(), 1);
  EXPECT_EQ(r.super_edges.size(), 1);
  EXPECT_TRUE(r.plus_corrections.empty());
  EXPECT_TRUE(r.minus_corrections.empty());
}

#include "mags/util_file.h"   // For io::read_from_file

TEST_F(GraphSummarizeTest, EmailEUCore) {
  // Path adjustment for build directory
  const std::string file_path = email_data;

  // 1. Establish Ground Truth
  Graph original;
  try {
      original = mags::io::read_from_file(file_path);
  } catch (const std::exception& e) {
      FAIL() << "Could not read file at " << file_path << ": " << e.what();
  }

  size_t original_edges = 0;
  for (const auto& adj : original) {
      original_edges += adj.size();
  }
  original_edges /= 2; // Undirected count

  // 2. Run Algorithm
  const auto r = summarize_from_file(file_path, 50, 40);

  // 3. Output Comparisons
  double ratio = (double)r.get_total_cost() / original_edges;

  // Convert to "Arc Equivalents" (Directed) for comparison with original codebase
  size_t original_arcs = original_edges * 2;
  size_t summary_arcs = r.get_total_cost() * 2;
  double ratio_arcs = (double)summary_arcs / original_arcs;

  std::cout << "==============================================" << std::endl;
  std::cout << "          COMPARISON REPORT                   " << std::endl;
  std::cout << "==============================================" << std::endl;
  std::cout << "[UNDIRECTED] (Your Implementation)" << std::endl;
  std::cout << "  Original Edges : " << original_edges << std::endl;
  std::cout << "  Summary Cost   : " << r.get_total_cost() << std::endl;
  std::cout << "  Ratio          : " << std::fixed << std::setprecision(4) << ratio << std::endl;
  std::cout << "----------------------------------------------" << std::endl;
  std::cout << "[ARC EQUIVALENT] (For comparison with Original MAGS)" << std::endl;
  std::cout << "  Original Arcs  : " << original_arcs << " (vs ~51,142)" << std::endl;
  std::cout << "  Summary Arcs   : " << summary_arcs  << " (vs ~42,827)" << std::endl;
  std::cout << "  Ratio          : " << ratio_arcs << " (vs 0.837)" << std::endl;
  std::cout << "==============================================" << std::endl;

  // Assertions
  EXPECT_GT(r.get_total_cost(), 0);
  EXPECT_LT(r.get_total_cost(), original_edges);
}

TEST_F(GraphSummarizeTest, EmailEUCore_DetailedRuntime) {
  const std::string file_path = email_data;

  // --- BASELINE VALUES (From Original Implementation) ---
  const double B_READ  = 0.013;
  const double B_MERGE = 0.227;
  const double B_ENC   = 0.001;
  const double B_RATIO = 0.837413476;

  // 1. Setup
  try {
      auto check = mags::io::read_from_file(file_path);
      if (check.empty()) GTEST_FAIL() << "Data file empty.";
  } catch (...) {
      GTEST_SKIP() << "Data file not found.";
  }

  using Clock = std::chrono::high_resolution_clock;
  using Duration = std::chrono::duration<double>;

  std::cout << "Mags Rewrite Benchmark vs Original (Baseline)" << std::endl;
  std::cout << "File: " << file_path << " | T=50, K=40" << std::endl;

  // --- 1. READ ---
  auto t_start = Clock::now();
  Graph input = mags::io::read_from_file(file_path);
  Graph clean = mags::preprocess::clean_graph(input);
  double s_read = Duration(Clock::now() - t_start).count();

  size_t original_edges = 0;
  for (const auto& adj : clean) original_edges += adj.size();
  original_edges /= 2;

  // --- 2. MERGE ---
  auto t_merge_start = Clock::now();
  auto candidates = mags::cg::generate_candidates(clean, 40);
  auto supernodes = mags::gm::greedy_merge(clean, 50, candidates);
  double s_merge = Duration(Clock::now() - t_merge_start).count();

  // --- 3. ENCODE ---
  auto t_enc_start = Clock::now();
  auto r = mags::out::output(clean, supernodes);
  double s_enc = Duration(Clock::now() - t_enc_start).count();

  // --- CALCULATIONS ---
  // Convert to arc equivalents for fair ratio comparison
  size_t my_summary_arcs = r.get_total_cost() * 2;
  size_t my_original_arcs = original_edges * 2;
  double my_ratio = (double)my_summary_arcs / my_original_arcs;

  // --- OUTPUT TABLE ---
  auto print_row = [](std::string metric, double my_val, double base_val, std::string unit) {
      double slower_by = my_val / base_val;
      std::cout << std::left << std::setw(15) << metric
                << "| " << std::setw(10) << my_val << unit
                << "| " << std::setw(10) << base_val << unit
                << "| " << std::setw(10) << std::fixed << std::setprecision(2) << slower_by << "x"
                << std::endl;
  };

  std::cout << "-------------------------------------------------------------" << std::endl;
  std::cout << std::left << std::setw(15) << "METRIC"
            << "| " << std::setw(11) << "REWRITE"
            << "| " << std::setw(11) << "ORIGINAL"
            << "| " << "SLOWER BY" << std::endl;
  std::cout << "-------------------------------------------------------------" << std::endl;

  print_row("Read Time", s_read, B_READ, "s");
  print_row("Merge Time", s_merge, B_MERGE, "s");
  print_row("Encode Time", s_enc, B_ENC, "s");

  std::cout << "-------------------------------------------------------------" << std::endl;
  std::cout << std::left << std::setw(15) << "COMPRESSION"
            << "| " << std::setw(11) << "REWRITE"
            << "| " << std::setw(11) << "ORIGINAL"
            << "| " << "IMPROVEMENT" << std::endl;
  std::cout << "-------------------------------------------------------------" << std::endl;

  double improvement = (1.0 - (my_ratio / B_RATIO)) * 100.0;

  std::cout << std::left << std::setw(15) << "Ratio"
            << "| " << std::setw(10) << std::setprecision(4) << my_ratio << " "
            << "| " << std::setw(10) << std::setprecision(4) << B_RATIO << " "
            << "| " << (B_RATIO - my_ratio) << " (" << improvement << "% )" << std::endl;

  std::cout << "-------------------------------------------------------------" << std::endl;
}