#include "options.h"

#include <tuple>
#include <utility>

template<size_t N>
struct fixed_string {
    char data[N];
    
    constexpr fixed_string(const char (&str)[N]) {
        std::copy_n(str, N, data);
    }
    
    constexpr operator const char*() const { 
      return data;
    }
};

template<typename... Opts>
class Parser {
  public:
    Parser() = default;
    template<typename... Args>
    requires (sizeof...(Args) == sizeof...(Opts))
    Parser(Args&&... opt)
      : _opt(std::forward<Args>(opt)...) {}
    void pars(const int argc, const char** argv);
    template<typename T, fixed_string Name, size_t Index = 0>
    T get();
    template<typename T>
    auto add(std::string long_name, std::string short_name = "", T default_val = T{});
  private:
    template<size_t Index = 0>
    int find(const std::string& flag, const char** argv, const int remain);
    std::tuple<Opts...> _opt;
};

template<typename Tuple>
struct TuplrToParser;

template<typename... Opts>
struct TuplrToParser<std::tuple<Opts...>> {
  using type = Parser<Opts...>;
};

#include "parser.cpp"
