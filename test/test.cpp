#include "vkg/base/base.hpp"
using namespace vkg;
auto main() -> int {
  WindowConfig windowConfig{};
  FeatureConfig featureConfig{};
  Base app{windowConfig, featureConfig};

  app.loop([&](double elapsed) {

  });
  return 0;
}