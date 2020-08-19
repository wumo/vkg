#pragma once

namespace vkg {
enum class DrawGroup : uint32_t {
  Unknown = ~0u,
  Unlit = 0u,
  BRDF,
  Reflective,
  Refractive,
  Transparent,
  Terrain,
  TransparentLines,
  OpaqueLines,
  Last = OpaqueLines
};
}