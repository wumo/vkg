#ifndef VKG_C_ATMOSPHERE_H
#define VKG_C_ATMOSPHERE_H

#include "c_vec.h"

#ifdef __cplusplus
extern "C" {
#else
  #include <stdbool.h>
#endif

struct CAtmosphereSetting;
typedef struct CAtmosphereSetting CAtmosphereSetting;

bool AtmosphereIsEnabled(CAtmosphereSetting *atmosphere);
void AtmosphereEnable(CAtmosphereSetting *atmosphere, bool enabled);

void AtmosphereGetSunDirection(
  CAtmosphereSetting *atmosphere, cvec3 *sunDirection, uint32_t offset_float);
void AtmosphereSetSunDirection(
  CAtmosphereSetting *atmosphere, cvec3 *sunDirection, uint32_t offset_float);

void AtmosphereGetEarthCenter(
  CAtmosphereSetting *atmosphere, cvec3 *earthCenter, uint32_t offset_float);
void AtmosphereSetEarthCenter(
  CAtmosphereSetting *atmosphere, cvec3 *earthCenter, uint32_t offset_float);

float AtmosphereGetSunIntensity(CAtmosphereSetting *atmosphere);
void AtmosphereSetSunIntensity(CAtmosphereSetting *atmosphere, float intensity);

float AtmosphereGetExposure(CAtmosphereSetting *atmosphere);
void AtmosphereSetExposure(CAtmosphereSetting *atmosphere, float exposure);

double AtmosphereGetSunAngularRadius(CAtmosphereSetting *atmosphere);
double AtmosphereGetSunSolidAngle(CAtmosphereSetting *atmosphere);
double AtmosphereGetLengthUnitInMeters(CAtmosphereSetting *atmosphere);
double AtmosphereGetBottomRadius(CAtmosphereSetting *atmosphere);

#ifdef __cplusplus
}
#endif

#endif //VKG_C_ATMOSPHERE_H
