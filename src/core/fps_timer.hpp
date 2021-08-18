#pragma once
#include "settings.hpp"

namespace core {
// Calculates the elapsed time from one time point to another and locks the
// frame time
class FPSTimer {
public:
  // The destructor is utilised to get the end time of a frame
  class DeltaTimer {
  public:
    DeltaTimer(FPSTimer &parent);
    ~DeltaTimer();

  private:
    FPSTimer &m_parent;
    // When did the frame start in seconds
    const float m_start_time;
  };

  friend class DeltaTimer;

  // max_fps .... The frame is locked to hold a steady fps count
  FPSTimer(const size_t max_fps);

  // Returns the time elapsed between the beginning and end of the last frame
  inline const float &get_delta_time() const { return m_delta_time; }

  // Starts a frame and returns a DeltaTimer which is used to end the frame
  // automatically when its scope ends
  DeltaTimer begin_frame();

private:
  // Handles everything that happens when a frame ends and locks the frame time
  void _end_frame();

  float m_delta_time;
  // At which frame time get the FPS locked in seconds
  const float m_min_delta_time;
};
} // namespace core