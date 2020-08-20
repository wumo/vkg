#pragma once
#include "vkg/base/base.hpp"
#include "vkg/math/glm_common.hpp"

namespace vkg {
class DensityProfileLayer {
public:
  DensityProfileLayer(): DensityProfileLayer(0.0, 0.0, 0.0, 0.0, 0.0) {}
  DensityProfileLayer(
    float width, float exp_term, float exp_scale, float linear_term, float constant_term)
    : width(width),
      exp_term(exp_term),
      exp_scale(exp_scale),
      linear_term(linear_term),
      constant_term(constant_term) {}
  float width;
  float exp_term;
  float exp_scale;
  float linear_term;
  float constant_term;
  float pad[3]{0};
};

struct DensityProfile {
  DensityProfileLayer layers[2];
};

using IrradianceSpectrum = glm::vec3;
using ScatteringSpectrum = glm::vec3;
using DimensionlessSpectrum = glm::vec3;
using Angle = float;
using Length = float;
struct AtmosphereParameters {
  IrradianceSpectrum solar_irradiance;
  Angle sun_angular_radius;
  ScatteringSpectrum rayleigh_scattering;
  Length bottom_radius;
  ScatteringSpectrum mie_scattering;
  Length top_radius;
  ScatteringSpectrum mie_extinction;
  float mie_phase_function_g;
  ScatteringSpectrum absorption_extinction;
  float mu_s_min;
  DimensionlessSpectrum ground_albedo;
  float pad;
  DensityProfile rayleigh_density;
  DensityProfile mie_density;
  DensityProfile absorption_density;
};

class AtmosphereModel {
  struct AtmosphereUniform {
    int transmittance_texture_width;
    int transmittance_texture_height;
    int scattering_texture_r_size;
    int scattering_texture_mu_size;
    int scattering_texture_mu_s_size;
    int scattering_texture_nu_size;
    int irradiance_texture_width;
    int irradiance_texture_height;
    glm::vec4 sky_spectral_radiance_to_luminance;
    glm::vec4 sun_spectral_radiance_to_luminance;
    AtmosphereParameters atmosphere;
  };
  struct SunUniform {
    glm::vec4 white_point{};
    glm::vec4 earth_center;
    glm::vec4 sun_direction;
    glm::vec2 sun_size;
    float exposure;
    float intensity{1};
  };

public:
  AtmosphereModel(
    Device &device, const std::vector<double> &wavelengths,
    const std::vector<double> &solar_irradiance, double sun_angular_radius,
    double bottom_radius, double top_radius,
    const std::vector<DensityProfileLayer> &rayleigh_density,
    const std::vector<double> &rayleigh_scattering,
    const std::vector<DensityProfileLayer> &mie_density,
    const std::vector<double> &mie_scattering, const std::vector<double> &mie_extinction,
    double mie_phase_function_g,
    const std::vector<DensityProfileLayer> &absorption_density,
    const std::vector<double> &absorption_extinction,
    const std::vector<double> &ground_albedo, double max_sun_zenith_angle,
    float length_unit_in_meters, unsigned int num_precomputed_wavelengths,
    float exposure_scale, glm::vec3 sunDirection, glm::vec3 earthCenter);

  AtmosphereModel(
    Device &device, double sun_angular_radius, double bottom_radius, double top_radius,
    float length_unit_in_meters, glm::vec3 sunDirection, glm::vec3 earthCenter);

  auto init(uint32_t num_scattering_orders = 4) -> void;

  auto atmosphereUBO() const -> BufferInfo;
  auto sunUBO() const -> BufferInfo;
  auto transmittanceTex() -> Texture &;
  auto scatteringTex() -> Texture &;
  auto irradianceTex() -> Texture &;

  auto updateSunIntesity(float intensity) -> void;
  auto updateExpsure(float exposure) -> void;
  auto updateSunDirection(glm::vec3 sunDirection) -> void;
  auto updateEarthCenter(glm::vec3 earthCenter) -> void;

private:
  auto initParameter(
    const std::vector<double> &wavelengths, const std::vector<double> &solar_irradiance,
    double sun_angular_radius, double bottom_radius_, double top_radius,
    const std::vector<DensityProfileLayer> &rayleigh_density,
    const std::vector<double> &rayleigh_scattering,
    const std::vector<DensityProfileLayer> &mie_density,
    const std::vector<double> &mie_scattering, const std::vector<double> &mie_extinction,
    double mie_phase_function_g,
    const std::vector<DensityProfileLayer> &absorption_density,
    const std::vector<double> &absorption_extinction,
    const std::vector<double> &ground_albedo, double max_sun_zenith_angle,
    float length_unit_in_meters_, unsigned int num_precomputed_wavelengths,
    float exposure_scale_, glm::vec3 sunDirection, glm::vec3 earthCenter) -> void;
  auto createDescriptors() -> void;

  auto createTransmittanceSets() -> void;
  auto recordTransmittanceCMD(vk::CommandBuffer cb) -> void;

  auto createDirectIrradianceSets() -> void;
  auto recordDirectIrradianceCMD(vk::CommandBuffer cb, vk::Bool32 cumulate) -> void;

  auto createSingleScatteringSets() -> void;
  auto recordSingleScatteringCMD(
    vk::CommandBuffer cb, const glm::mat4 &luminance_from_radiance, vk::Bool32 cumulate)
    -> void;

  auto createScatteringDensitySets() -> void;
  auto recordScatteringDensityCMD(vk::CommandBuffer cb, int32_t scatteringOrder) -> void;

  auto createIndirectIrradianceSets() -> void;
  auto recordIndirectIrradianceCMD(
    vk::CommandBuffer cb, const glm::mat4 &luminance_from_radiance,
    int32_t scatteringOrder) -> void;

  auto createMultipleScatteringSets() -> void;
  auto recordMultipleScatteringCMD(
    vk::CommandBuffer cb, const glm::mat4 &luminance_from_radiance) -> void;
  auto compute(uint32_t num_scattering_orders) -> void;
  auto precompute(
    const glm::vec3 &lambdas, const glm::mat4 &luminance_from_radiance, bool cumulate,
    uint32_t num_scattering_orders) -> void;

private:
  Device &device;

  uint32_t num_precomputed_wavelengths_;
  std::unique_ptr<Texture> transmittanceTex_, scatteringTex_, irradianceTex_;

  std::unique_ptr<Texture> deltaIrradianceTex, deltaRayleighScatteringTex,
    deltaMieScatteringTex, deltaScatteringDensityTex;

  std::unique_ptr<Buffer> atmosphereUBO_, sunUBO_;

  bool do_white_balance_{true};
  double bottom_radius;
  float length_unit_in_meters;
  float exposure_{10.0f};
  float exposure_scale{1e-5f};

  std::function<AtmosphereParameters(const glm::vec3 &)> calcAtmosphereParams;

  vk::UniqueDescriptorPool descriptorPool;

  struct TransmittanceSetDef: DescriptorSetDef {
    __uniform__(atmosphere, vk::ShaderStageFlagBits::eCompute);
    __image2D__(transmittance, vk::ShaderStageFlagBits::eCompute);
  } transmittanceSetDef;
  struct TransmittancePipeDef: PipelineLayoutDef {
    __set__(set, TransmittanceSetDef);
  } transmittancePipeDef;
  vk::DescriptorSet transmittanceSet;
  vk::UniquePipeline transmittancePipe;

  struct DirectIrradianceSetDef: DescriptorSetDef {
    __uniform__(atmosphere, vk::ShaderStageFlagBits::eCompute);
    __sampler2D__(transmittance, vk::ShaderStageFlagBits::eCompute);
    __image2D__(delta_irradiance, vk::ShaderStageFlagBits::eCompute);
    __image2D__(irradiance, vk::ShaderStageFlagBits::eCompute);
  } directIrradianceSetDef;
  struct DirectIrradiancePipeDef: PipelineLayoutDef {
    __push_constant__(constant, vk::ShaderStageFlagBits::eCompute, vk::Bool32);
    __set__(set, DirectIrradianceSetDef);
  } directIrradiancePipeDef;
  vk::DescriptorSet directIrradianceSet;
  vk::UniquePipeline directIrradiancePipe;

  struct SingleScatteringSetDef: DescriptorSetDef {
    __uniform__(atmosphere, vk::ShaderStageFlagBits::eCompute);
    __sampler2D__(transmittance, vk::ShaderStageFlagBits::eCompute);
    __image3D__(delta_rayleigh, vk::ShaderStageFlagBits::eCompute);
    __image3D__(delta_mie, vk::ShaderStageFlagBits::eCompute);
    __image3D__(scattering, vk::ShaderStageFlagBits::eCompute);
  } singleScatteringSetDef;
  struct CumulateLFUConstant {
    glm::mat4 luminance_from_radiance;
    vk::Bool32 cumulate;
  };
  struct SingleScatteringLayoutDef: PipelineLayoutDef {
    __push_constant__(constant, vk::ShaderStageFlagBits::eCompute, CumulateLFUConstant);
    __set__(set, SingleScatteringSetDef);
  } singleScatteringPipeDef;
  vk::DescriptorSet singleScatteringSet;
  vk::UniquePipeline singleScatteringPipe;

  struct ScatteringDensitySetDef: DescriptorSetDef {
    __uniform__(atmosphere, vk::ShaderStageFlagBits::eCompute);
    __sampler2D__(transmittance, vk::ShaderStageFlagBits::eCompute);
    __sampler3D__(delta_rayleigh, vk::ShaderStageFlagBits::eCompute);
    __sampler3D__(delta_mie, vk::ShaderStageFlagBits::eCompute);
    __sampler3D__(multpli_scattering, vk::ShaderStageFlagBits::eCompute);
    __sampler2D__(irradiance, vk::ShaderStageFlagBits::eCompute);
    __image3D__(scattering_density, vk::ShaderStageFlagBits::eCompute);
  } scatteringDensitySetDef;
  struct ScatteringDensityLayoutDef: PipelineLayoutDef {
    __push_constant__(constant, vk::ShaderStageFlagBits::eCompute, int32_t);
    __set__(set, ScatteringDensitySetDef);
  } scatteringDensityPipeDef;
  vk::DescriptorSet scatteringDensitySet;
  vk::UniquePipeline scatteringDensityPipe;

  struct IndirectIrradianceSetDef: DescriptorSetDef {
    __uniform__(atmosphere, vk::ShaderStageFlagBits::eCompute);
    __sampler3D__(delta_rayleigh, vk::ShaderStageFlagBits::eCompute);
    __sampler3D__(delta_mie, vk::ShaderStageFlagBits::eCompute);
    __sampler3D__(multpli_scattering, vk::ShaderStageFlagBits::eCompute);
    __image2D__(delta_irradiance, vk::ShaderStageFlagBits::eCompute);
    __image2D__(irradiance, vk::ShaderStageFlagBits::eCompute);
  } indirectIrradianceSetDef;
  struct ScatteringOrderLFUConstant {
    glm::mat4 luminance_from_radiance;
    int32_t scattering_order;
  };
  struct IndirectIrradianceLayoutDef: PipelineLayoutDef {
    __push_constant__(
      constant, vk::ShaderStageFlagBits::eCompute, ScatteringOrderLFUConstant);
    __set__(set, IndirectIrradianceSetDef);
  } indirectIrradiancePipeDef;
  vk::DescriptorSet indirectIrradianceSet;
  vk::UniquePipeline indirectIrradiancePipe;

  struct MultipleScatteringSetDef: DescriptorSetDef {
    __uniform__(atmosphere, vk::ShaderStageFlagBits::eCompute);
    __sampler2D__(transmittance, vk::ShaderStageFlagBits::eCompute);
    __sampler3D__(scattering_density, vk::ShaderStageFlagBits::eCompute);
    __image3D__(delta_multiple_scattering, vk::ShaderStageFlagBits::eCompute);
    __image3D__(scattering, vk::ShaderStageFlagBits::eCompute);
  } multipleScatteringSetDef;
  struct MultipleScatteringLayoutDef: PipelineLayoutDef {
    __push_constant__(constant, vk::ShaderStageFlagBits::eCompute, glm::mat4);
    __set__(set, MultipleScatteringSetDef);
  } multipleScatteringPipeDef;
  vk::DescriptorSet multipleScatteringSet;
  vk::UniquePipeline multipleScatteringPipe;
};
}