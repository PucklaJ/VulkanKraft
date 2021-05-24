#include "log.hpp"

namespace core {
void Log::_log(const char *msg_type, std::ostream &stream,
               const std::string &msg) {
  stream << start_type << msg_type << end_type << msg << std::endl;
}
} // namespace core