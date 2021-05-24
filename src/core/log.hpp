#pragma once
#include <iostream>
#include <string>

namespace core {
class Log {
public:
  static inline void info(const std::string &msg) {
    _log(info_type, std::cout, msg);
  }
  static inline void warning(const std::string &msg) {
    _log(warning_type, std::cerr, msg);
  }
  static inline void error(const std::string &msg) {
    _log(error_type, std::cerr, msg);
  }

private:
  static constexpr char start_type[] = "[";
  static constexpr char end_type[] = "] ";
  static constexpr char info_type[] = "\033[1;34mINFO\033[0m";
  static constexpr char warning_type[] = "\033[1;33mWARN\033[0m";
  static constexpr char error_type[] = "\033[1;31mERROR\033[0m";

  static void _log(const char *msg_type, std::ostream &stream,
                   const std::string &msg);
};
} // namespace core
