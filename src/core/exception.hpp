#pragma once

#include <exception>
#include <string>

namespace core {
class VulkanKraftException : public std::exception {
public:
  VulkanKraftException(const std::string &msg) noexcept : m_msg(msg) {}
  const char *what() const noexcept override { return m_msg.c_str(); }

private:
  std::string m_msg;
};
} // namespace core