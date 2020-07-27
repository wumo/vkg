#pragma once
#include <string>
namespace vkg {
struct WindowConfig {
  std::string title{"Window"};
  uint32_t width{1960}, height{1080};
  bool fullscreen{false};
  bool vsync{false};
};

struct FeatureConfig {
  bool rayTracing{false};
};
}