#include <stdexcept>
#include <string>
#include <type_traits>

template<typename T>
T from_string(const std::string& v) {
  static_assert(std::is_same_v<T, std::string>,
                "Unsupported type add specification");
  return v;
}

template<>
float from_string<float>(const std::string& v) {
  return std::stof(v);
}

template<>
long from_string<long>(const std::string& v) {
  return std::stol(v);
}

template<>
long long from_string<long long>(const std::string& v) {
  return std::stoll(v);
}

template<>
unsigned long from_string<unsigned long>(const std::string& v) {
  return std::stoul(v);
}

template<>
unsigned long long from_string<unsigned long long>(const std::string& v) {
  return std::stoull(v);
}

template<>
int from_string<int>(const std::string& v) {
  return std::stoi(v);
}

template<>
double from_string<double>(const std::string& v) {
  return std::stod(v);
}

template<typename... Opts>
template<size_t Index>
int Parser<Opts...>::find(const std::string& flag, const char** argv, const int remain) {
  if constexpr (Index < sizeof...(Opts)) {
    auto& next = std::get<Index>(_opt);
    if (next.long_name == flag || next.short_name == flag) {
      if (next.arg_count > remain) {
        throw std::runtime_error("Not enough arguments for option: " + flag);
      }
      if constexpr (next.arg_count == 1) {
        next.value = from_string<decltype(next.value)>(argv[0]);
      } else {
        for (int i = 0; i < next.arg_count; i++) {
          next.value.push_back(from_string<decltype(next.value)::value_type>(argv[i]));
        }
      }
      next.is_set = true;
      return next.arg_count;
    } else {
      return find<Index + 1>(flag, argv, remain);
    }
  } else {
    throw std::runtime_error("unknown option: " + flag);
  }
}

template<typename... Opts>
void Parser<Opts...>::pars(const int argc, const char** argv) {
  for (int i = 1; i < argc; i++) {
    std::string token = argv[i];
    if (token.starts_with("--") || token.starts_with("-")) {
      i += find(token, &argv[i + 1], argc - i - 1);
    } else {
      throw std::runtime_error("unknown option: " + token);
    }
  }
}

template<typename... Opts>
template<typename T, fixed_string Name, size_t Index>
T Parser<Opts...>::get() {
  if constexpr (Index >= sizeof...(Opts)) {
      throw std::runtime_error("Option not found: " + std::string(Name.data));
  } else {
      auto& opt = std::get<Index>(_opt);
      if constexpr (std::is_convertible_v<decltype(opt.value), T>) {
          if (opt.long_name == Name.data || opt.short_name == Name.data) {
              return opt.value;
          }
      }
      return get<T, Name, Index + 1>();
  }
}

template<typename... Opts>
template<typename T>
auto Parser<Opts...>::add(std::string long_name, std::string short_name, T default_val) {
  Options<T> new_opt(long_name, short_name);
  new_opt.value = default_val;
  auto new_turple = std::tuple_cat(_opt, std::make_tuple(new_opt));
  using new_parser = typename TuplrToParser<decltype(new_turple)>::type;
  return std::apply([](auto&&... opts) {
    return new_parser(std::forward<decltype(opts)>(opts)...);
  });
}
