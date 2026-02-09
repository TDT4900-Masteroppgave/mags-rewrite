#include "mags/candidate_generation.h"
#include "mags/util_time.h"
#include "mags/graph_summarize.h"
#include <chrono>
#include <iomanip>
#include <iostream>
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

  const std::string dataset_path = argv[1];
  int t = 50;
  int k = 40;
  std::string headers = "program, dataset, hash_num, iterations, read, merge, encoding, relative_size";
  const std::string program = "mags_rewrite";
  const int hash_num = cg::H_FUNCS;

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

  // Execution
  try {
    // Run the pipeline
    auto rep = summarize_from_file(dataset_path, t, k);

    // Generating report
    std::cout << headers << std::endl;
    std::cout << program << ", " << dataset_path << ", " << hash_num << ", " << t << ", " 
              << std::fixed << std::setprecision(3) // sets precision for all subsequent floating-point output
              << timing::get_elapsed_time(timing::start_time, timing::read_time).count() << ", " 
              << timing::get_elapsed_time(timing::read_time, timing::merge_time).count() << ", " 
              << timing::get_elapsed_time(timing::merge_time, timing::encoding_time).count() << ", "
              << std::defaultfloat << std::setprecision(9) // resets precision
              << rep.get_total_cost() 
              << std::endl;

  } catch (const std::exception &e) {
    std::cerr << "\n[ERROR] Summarization failed: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}