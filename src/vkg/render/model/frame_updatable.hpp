#pragma once
#include <cstdint>
#include "vkg/render/ranges.hpp"

namespace vkg {
class FrameUpdatable {
  friend class SceneSetupPass;

protected:
  virtual void updateDesc(uint32_t frameIdx) = 0;
  /**ticket for scheduled update*/
  uint32_t ticket{nullIdx};
};
}
