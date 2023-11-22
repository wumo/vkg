
#include <cfloat>
#include "fps_meter.hpp"

namespace vkg {
auto FPSMeter::update(const double dt) -> void {
    fpsAccumulator += dt - fpsHistory[historyPointer];
    fpsHistory[historyPointer] = dt;
    historyPointer = (historyPointer + 1) % kFPSHistorySize;
    fps_ = (fpsAccumulator > 0.0f) ? (1000.0f / (fpsAccumulator / static_cast<float>(kFPSHistorySize))) : DBL_MAX;
}

auto FPSMeter::fps() const -> int { return int(fps_); }

auto FPSMeter::frameTime() const -> double { return 1000.0f / fps_; }
}