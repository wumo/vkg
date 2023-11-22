#include "c_fpsmeter.h"
#include "vkg/util/fps_meter.hpp"
using namespace vkg;
uint32_t FPSMeterGetFPS(CFPSMeter *fpsMeter) {
    auto *fpsMeter_ = reinterpret_cast<FPSMeter *>(fpsMeter);
    return fpsMeter_->fps();
}
double FPSMeterGetFrameTime(CFPSMeter *fpsMeter) {
    auto *fpsMeter_ = reinterpret_cast<FPSMeter *>(fpsMeter);
    return fpsMeter_->frameTime();
}
