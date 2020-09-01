#version 460
#extension GL_NV_ray_tracing : require
#extension GL_GOOGLE_include_directive : require

#define SHADING_BRDF
#define USE_ATMOSPHERE
#include "rchit.h"
