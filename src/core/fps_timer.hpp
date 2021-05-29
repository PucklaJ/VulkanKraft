#pragma once
#include "settings.hpp"

namespace core {
class FPSTimer {
public:
  class DeltaTimer {
  public:
    DeltaTimer(FPSTimer &parent);
    ~DeltaTimer();

  private:
    FPSTimer &m_parent;
    const float m_start_time;
  };

  friend class DeltaTimer;

  FPSTimer(const Settings &settings);

  inline const float &get_delta_time() const { return m_delta_time; }

  DeltaTimer begin_frame();

private:
  void _end_frame();

  float m_delta_time;
  const float m_min_delta_time;
};
} // namespace core