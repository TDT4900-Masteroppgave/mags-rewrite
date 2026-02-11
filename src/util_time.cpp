#include "mags/util_time.h"
#include <chrono>

std::chrono::system_clock::time_point timing::start_time{};
std::chrono::system_clock::time_point timing::read_time{};
std::chrono::system_clock::time_point timing::merge_time{};
std::chrono::system_clock::time_point timing::encoding_time{};

void timing::set_time(std::chrono::system_clock::time_point& time) {
  time = std::chrono::system_clock::now();
}

std::chrono::duration<double> timing::get_elapsed_time(std::chrono::system_clock::time_point time_from) {
  auto now = std::chrono::system_clock::now();
  return std::chrono::duration<double>(now - time_from);
}   

std::chrono::duration<double> timing::get_elapsed_time(
    std::chrono::system_clock::time_point time_from,
    std::chrono::system_clock::time_point time_to) {
  return std::chrono::duration<double>(time_to - time_from);
}   