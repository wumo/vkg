#ifndef ATMOSPHERE_H
#define ATMOSPHERE_H

#ifndef ATMOSPHERE_SET
  #define ATMOSPHERE_SET 0
#endif

#define IN(x) const in x
#define OUT(x) out x
#define TEMPLATE(x)
#define TEMPLATE_ARGUMENT(x)
#define assert(x)
#define COMBINED_SCATTERING_TEXTURES

#include "definitions.h"

layout(set = ATMOSPHERE_SET, binding = 0) uniform AtmosphereUniform {
  int TRANSMITTANCE_TEXTURE_WIDTH;
  int TRANSMITTANCE_TEXTURE_HEIGHT;
  int SCATTERING_TEXTURE_R_SIZE;
  int SCATTERING_TEXTURE_MU_SIZE;
  int SCATTERING_TEXTURE_MU_S_SIZE;
  int SCATTERING_TEXTURE_NU_SIZE;
  int IRRADIANCE_TEXTURE_WIDTH;
  int IRRADIANCE_TEXTURE_HEIGHT;
  vec3 SKY_SPECTRAL_RADIANCE_TO_LUMINANCE;
  vec3 SUN_SPECTRAL_RADIANCE_TO_LUMINANCE;
  AtmosphereParameters ATMOSPHERE;
};

#include "functions.h"

#endif