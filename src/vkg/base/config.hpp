#pragma once
#include <string>
namespace vkg {
struct WindowConfig {
  std::string title{"Window"};
  uint32_t width{1960}, height{1080};
};

struct FeatureConfig {
  bool fullscreen{false};
  bool vsync{false};
  uint32_t numFrames{1};
  bool rayTrace{false};
};
}