#include <chrono>

struct timing {
  static std::chrono::system_clock::time_point start_time;
  static std::chrono::system_clock::time_point read_time;
  static std::chrono::system_clock::time_point merge_time;
  static std::chrono::system_clock::time_point encoding_time;

  static void set_time(std::chrono::system_clock::time_point time);
  static std::chrono::duration<double> get_elapsed_time(std::chrono::system_clock::time_point time_from);
  static std::chrono::duration<double> get_elapsed_time(std::chrono::system_clock::time_point time_from, std::chrono::system_clock::time_point time_to);
};