#pragma once
#include "screen_pass.hpp"

namespace vkg {
struct ToneMapPushConstant {
  float exposure{1.0};
};
class ToneMapPass: public ScreenPass<ToneMapPushConstant> {
public:
  ToneMapPass();

private:
  void updatePushConstant() override;
};
}
