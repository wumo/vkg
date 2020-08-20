#include "tonemap_pass.hpp"
#include "postprocess/tonemap_comp.hpp"

namespace vkg {
ToneMapPass::ToneMapPass(): ScreenPass(shader::postprocess::tonemap_comp_span) {}
void ToneMapPass::updatePushConstant() { pushConstant.exposure = 1.0; }
}