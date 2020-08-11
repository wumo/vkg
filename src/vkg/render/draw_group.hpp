#pragma once

namespace vkg {
enum class DrawGroup : uint32_t {
  Unknown = ~0u,
  Unlit = 0u,
  BRDF,
  TransparentLines,
  OpaqueLines,
  Reflective,
  Refractive,
  Transparent,
  Terrain,
  Last = Terrain
};
}