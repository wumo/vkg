#version 460
#extension GL_NV_ray_tracing : require
#extension GL_GOOGLE_include_directive : require

#define SHADING_REFLECTIVE
#define SHADING_REFRACTIVE
#include "rchit.h"
