#pragma once
#include <iostream>
#include <string>

namespace core {
// Handles logging to stdout
class Log {
public:
  // Prints a general information string to stdout
  static inline void info(const std::string &msg) {
    _log(info_type, std::cout, msg);
  }
  // Prints a warning string to stderr
  static inline void warning(const std::string &msg) {
    _log(warning_type, std::cerr, msg);
  }
  // Prints an error string to stderr
  static inline void error(const std::string &msg) {
    _log(error_type, std::cerr, msg);
  }

  static inline void debug(const std::string &msg) {
    _log(debug_type, std::cout, msg);
  }

private:
  // Variables used to decorate the printed message
  static constexpr char start_type[] = "[";
  static constexpr char end_type[] = "] ";
  static constexpr char info_type[] = "\033[1;34mINFO\033[0m";
  static constexpr char warning_type[] = "\033[1;33mWARN\033[0m";
  static constexpr char error_type[] = "\033[1;31mERROR\033[0m";
  static constexpr char debug_type[] = "\033[1;32mDEBUG\033[0m";

  // Prints msg to stream using msg_type as decoration
  static void _log(const char *msg_type, std::ostream &stream,
                   const std::string &msg);
};
} // namespace core
