#pragma once
#include "screen_pass.hpp"
#include "postprocess/fxaa_frag.hpp"

namespace vkg {
struct FxaaPushConstant {
  float fxaaQualitySubpix{0.75};
  float fxaaQualityEdgeThreshold{0.125};
  float fxaaQualityEdgeThresholdMin{0.0625};
};
class FxaaPass: public ScreenPass<FxaaPushConstant> {
public:
  FxaaPass(): ScreenPass(shader::postprocess::fxaa_frag_span) {}

protected:
  void updatePushConstant() override{

  };
};
}
