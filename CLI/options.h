#include <string>
#include <vector>
#include <algorithm>

template <typename T, size_t N = 1>
struct Options {
  Options(std::string long_n, std::string short_n = "") 
  : long_name(long_n), short_name(short_n) {}
  std::string long_name;
  std::string short_name;
  std::string description;
  std::conditional_t<N == 1, T, std::vector<T>> value{};
  bool is_set = false;
  static constexpr size_t arg_count = N;
};