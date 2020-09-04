#ifndef VKG_C_FPSMETER_HPP
#define VKG_C_FPSMETER_HPP

#include <cstdint>
#include "c_vec.h"
#ifdef __cplusplus
extern "C" {
#endif

struct CFPSMeter;
typedef struct CFPSMeter CFPSMeter;

uint32_t FPSMeterGetFPS(CFPSMeter *fpsMeter);
double FPSMeterGetFrameTime(CFPSMeter *fpsMeter);

#ifdef __cplusplus
}
#endif
#endif //VKG_C_FPSMETER_HPP
