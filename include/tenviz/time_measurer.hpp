#pragma once

#include <chrono>

namespace tenviz {
/**
 * Simple time measurement.
 */
class TimeMeasurer {
 public:
  TimeMeasurer();

  /**
   * Returns the elapsed time since the last call to this function and
   * reset the time.
   *
   * @return The elapsed time in seconds.
   */
  double Tick();

  /**
   * Returns the elapsed time since the last call to TimeMeasurer::Tick
   *
   * @return The elapsed time in seconds.
   */
  double GetElapsed() const;

 private:
  void GetElapsedAndNow(double &elapsed,
                        std::chrono::steady_clock::time_point &now) const;

  std::chrono::steady_clock::time_point last_tick_;
};
}  // namespace tenviz
