#pragma once
#include <cstdint>

namespace vkg {
const uint32_t nullIdx = ~0U;

struct UIntRange {

  uint32_t start{nullIdx};
  uint32_t size{0};

  auto endExclusive() const -> uint32_t { return start + size; }
};
}