#ifndef VKG_TEXTURE_FORMATS_H
#define VKG_TEXTURE_FORMATS_H
#include <cstdint>
#ifdef __cplusplus
extern "C" {
#endif

enum TextureFormat {
  R8Unorm,
  R16Sfloat,
  R32Sfloat,
  R8G8B8A8Unorm,
  R16G16B16A16Sfloat,
  R32G32B32A32Sfloat
};

#ifdef __cplusplus
}
#endif
#endif //VKG_TEXTURE_FORMATS_H
