#pragma once
#include "screen_pass.hpp"
#include "postprocess/tonemap_comp.hpp"

namespace vkg {
struct ToneMapPushConstant {
  float exposure{1.0};
};
class ToneMapPass: public ScreenPass<ToneMapPushConstant> {
public:
  ToneMapPass(): ScreenPass(shader::postprocess::tonemap_comp_span) {}

private:
  void updatePushConstant() override { pushConstant.exposure = 1.0; }
};
}
