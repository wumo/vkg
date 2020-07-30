#version 460
#extension GL_NV_ray_tracing : require
#extension GL_GOOGLE_include_directive : require

#define USE_ATMOSPHERE
#define REFLECT_TRACE
#define REFRACT_TRACE
#include "lighting.h"
