#pragma once

#include <cstddef>
namespace vkg {
class FPSMeter {
public:
  void update(double dt);
  auto fps() const -> int;
  auto frameTime() const -> double;

private:
  static const size_t kFPSHistorySize{128};

  double fpsHistory[kFPSHistorySize]{0.0f};
  size_t historyPointer{0};
  double fpsAccumulator{0.0f};
  double fps_{0.0f};
};
}
