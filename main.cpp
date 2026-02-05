#include "graph_summarize.h"
#include <chrono>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>

void print_usage(const char *prog_name) {
  std::cerr << "Usage: " << prog_name << " <graph_file_path> [t] [k]\n"
            << "Options:\n"
            << "  t   Merge threshold (default: 50)\n"
            << "  k   Candidate set size (default: 40)\n"
            << "Example:\n"
            << "  " << prog_name << " data/graph.txt 50 40" << std::endl;
}

int main(int argc, char *argv[]) {
  // 1. Argument Parsing
  if (argc < 2) {
    print_usage(argv[0]);
    return 1;
  }

  const std::string path = argv[1];
  int t = 50;
  int k = 40;

  try {
    if (argc >= 3)
      t = std::stoi(argv[2]);
    if (argc >= 4)
      k = std::stoi(argv[3]);
  } catch ([[maybe_unused]] const std::exception &e) {
    std::cerr << "Error: Invalid number format for parameters t or k."
              << std::endl;
    return 1;
  }

  // 2. Initial Print
  std::cout << "==============================================" << std::endl;
  std::cout << "          MAGS Graph Summarization            " << std::endl;
  std::cout << "==============================================" << std::endl;
  std::cout << "Input File : " << path << std::endl;
  std::cout << "Parameters : T=" << t << ", K=" << k << std::endl;
  std::cout << "----------------------------------------------" << std::endl;
  std::cout << "[INFO] Reading and summarising graph..." << std::endl;

  // 3. Execution & Timing
  auto start_time = std::chrono::high_resolution_clock::now();

  try {
    // Run the pipeline
    auto rep = graph_summarize(path, t, k);

    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end_time - start_time;

    // 4. Final Output Report
    std::cout << "[INFO] Done!" << std::endl;
    std::cout << "==============================================" << std::endl;
    std::cout << "               FINAL REPORT                   " << std::endl;
    std::cout << "==============================================" << std::endl;

    // Breakdown of the cost
    size_t cost_super = rep.super_edges.size();
    size_t cost_plus = rep.plus_corrections.size();
    size_t cost_minus = rep.minus_corrections.size();
    size_t total_cost = rep.get_total_cost();

    std::cout << std::left << std::setw(25) << "Execution Time" << ": "
              << std::fixed << std::setprecision(4) << elapsed.count() << " s"
              << std::endl;

    std::cout << std::left << std::setw(25) << "Total Summary Cost" << ": "
              << total_cost << " edges" << std::endl;

    std::cout << "----------------------------------------------" << std::endl;
    std::cout << "Structure Breakdown:" << std::endl;
    std::cout << "  [S] Super-Edges       : " << cost_super << std::endl;
    std::cout << "  [+] Plus Corrections  : " << cost_plus << std::endl;
    std::cout << "  [-] Minus Corrections : " << cost_minus << std::endl;
    std::cout << "==============================================" << std::endl;

  } catch (const std::exception &e) {
    std::cerr << "\n[ERROR] Summarization failed: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}