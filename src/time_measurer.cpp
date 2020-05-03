#include "time_measurer.hpp"

using namespace std;

namespace tenviz {
TimeMeasurer::TimeMeasurer() { last_tick_ = chrono::steady_clock::now(); }

void TimeMeasurer::GetElapsedAndNow(
    double &elapsed, chrono::steady_clock::time_point &now) const {
  now = chrono::steady_clock::now();
  elapsed = static_cast<double>(
      chrono::duration_cast<chrono::microseconds>(now - last_tick_).count());
  elapsed *= 1.0e-5;
}

double TimeMeasurer::Tick() {
  double elapsed;
  chrono::steady_clock::time_point now;
  GetElapsedAndNow(elapsed, now);
  last_tick_ = now;
  return elapsed;
}

double TimeMeasurer::GetElapsed() const {
  double elapsed;
  chrono::steady_clock::time_point now;
  GetElapsedAndNow(elapsed, now);
  return elapsed;
}
}  // namespace tenviz
