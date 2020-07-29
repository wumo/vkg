#pragma once
#include <cstdint>

namespace vkg {
template<typename T>
struct Allocation {
  uint32_t offset;
  T *ptr;
};
}