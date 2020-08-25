#pragma once

class CallFrameUpdater {
public:
  virtual void update(uint32_t frameIdx, double elapsedDuration) {}
};
