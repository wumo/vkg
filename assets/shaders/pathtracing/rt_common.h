#ifndef VKG_PATHTRACING_COMMON_H
#define VKG_PATHTRACING_COMMON_H

#include "../common.h"
#include "../tonemap.h"
#include "../math.h"
#include "../raytracing/rt_common.h"

const uint FLAG_NONE = 0;
const uint FLAG_INSIDE = 1;
const uint FLAG_DONE = 2;
const uint FLAG_FIRST_PATH_SEGMENT = 4;
const uint FLAG_PRIMARY_TEST = 8;

const uint MATERIAL_FLAG_NONE = 0;
const uint MATERIAL_FLAG_OPAQUE = 1;       // allows to skip opacity evaluation
const uint MATERIAL_FLAG_DOUBLE_SIDED = 2; // geometry is only visible from the front side

// clang-format off
void add_flag(inout uint flags, uint to_add) { flags |= to_add; }
void toggle_flag(inout uint flags, uint to_toggle) { flags ^= to_toggle; }
void remove_flag(inout uint flags, uint to_remove) {flags &= ~to_remove; }
bool has_flag(uint flags, uint to_check) { return (flags & to_check) != 0; }
// clang-format on

struct PathTracingRayPayload {
  vec3 contribution;
  vec3 weight;
  vec3 rayOrigin;
  vec3 rayDir;
  uint seed;
  float last_pdf;
  uint flags;
  float hitT;
};

// Payload for Shadow
struct PathTracingShadowRayPayload {
  bool isHit;
  uint seed;
};

#endif //VKG_DEFERRED_COMMON_H
