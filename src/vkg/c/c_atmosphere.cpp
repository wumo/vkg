#include "c_atmosphere.h"
#include "vkg/render/model/atmosphere.hpp"
#include <cstring>
using namespace vkg;
bool AtmosphereIsEnabled(CAtmosphereSetting *atmosphere) {
    auto *atmospher_ = reinterpret_cast<AtmosphereSetting *>(atmosphere);
    return atmospher_->isEnabled();
}
void AtmosphereEnable(CAtmosphereSetting *atmosphere, bool enabled) {
    auto *atmospher_ = reinterpret_cast<AtmosphereSetting *>(atmosphere);
    atmospher_->enable(enabled);
}
void AtmosphereGetSunDirection(CAtmosphereSetting *atmosphere, cvec3 *sunDirection, uint32_t offset_float) {
    auto *atmospher_ = reinterpret_cast<AtmosphereSetting *>(atmosphere);
    auto sunDir = atmospher_->sunDirection();
    memcpy((float *)sunDirection + offset_float, &sunDir, sizeof(sunDir));
}
void AtmosphereSetSunDirection(CAtmosphereSetting *atmosphere, cvec3 *sunDirection, uint32_t offset_float) {
    auto *atmospher_ = reinterpret_cast<AtmosphereSetting *>(atmosphere);
    atmospher_->setSunDirection(*(glm::vec3 *)((float *)sunDirection + offset_float));
}
void AtmosphereGetEarthCenter(CAtmosphereSetting *atmosphere, cvec3 *earthCenter, uint32_t offset_float) {
    auto *atmospher_ = reinterpret_cast<AtmosphereSetting *>(atmosphere);
    auto earthCenter_ = atmospher_->earthCenter();
    memcpy((float *)earthCenter + offset_float, &earthCenter_, sizeof(earthCenter_));
}
void AtmosphereSetEarthCenter(CAtmosphereSetting *atmosphere, cvec3 *earthCenter, uint32_t offset_float) {
    auto *atmospher_ = reinterpret_cast<AtmosphereSetting *>(atmosphere);
    atmospher_->setEarthCenter(*(glm::vec3 *)((float *)earthCenter + offset_float));
}
float AtmosphereGetSunIntensity(CAtmosphereSetting *atmosphere) {
    auto *atmospher_ = reinterpret_cast<AtmosphereSetting *>(atmosphere);
    return atmospher_->sunIntensity();
}
void AtmosphereSetSunIntensity(CAtmosphereSetting *atmosphere, float intensity) {
    auto *atmospher_ = reinterpret_cast<AtmosphereSetting *>(atmosphere);
    atmospher_->setSunIntensity(intensity);
}
float AtmosphereGetExposure(CAtmosphereSetting *atmosphere) {
    auto *atmospher_ = reinterpret_cast<AtmosphereSetting *>(atmosphere);
    return atmospher_->exposure();
}
void AtmosphereSetExposure(CAtmosphereSetting *atmosphere, float exposure) {
    auto *atmospher_ = reinterpret_cast<AtmosphereSetting *>(atmosphere);
    atmospher_->setExposure(exposure);
}
double AtmosphereGetSunAngularRadius(CAtmosphereSetting *atmosphere) {
    auto *atmospher_ = reinterpret_cast<AtmosphereSetting *>(atmosphere);
    return atmospher_->sunAngularRadius();
}
double AtmosphereGetSunSolidAngle(CAtmosphereSetting *atmosphere) {
    auto *atmospher_ = reinterpret_cast<AtmosphereSetting *>(atmosphere);
    return atmospher_->sunSolidAngle();
}
double AtmosphereGetLengthUnitInMeters(CAtmosphereSetting *atmosphere) {
    auto *atmospher_ = reinterpret_cast<AtmosphereSetting *>(atmosphere);
    return atmospher_->lengthUnitInMeters();
}
double AtmosphereGetBottomRadius(CAtmosphereSetting *atmosphere) {
    auto *atmospher_ = reinterpret_cast<AtmosphereSetting *>(atmosphere);
    return atmospher_->bottomRadius();
}
