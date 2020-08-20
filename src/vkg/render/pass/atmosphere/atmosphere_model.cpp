#include "atmosphere_model.hpp"
#include "constants.hpp"

namespace vkg {
namespace {
constexpr double kLambdaR = 680.0;
constexpr double kLambdaG = 550.0;
constexpr double kLambdaB = 440.0;

constexpr int kLambdaMin = 360;
constexpr int kLambdaMax = 830;

auto CieColorMatchingFunctionTableValue(double wavelength, int column) -> double {
  if(wavelength <= kLambdaMin || wavelength >= kLambdaMax) { return 0.0; }
  double u = (wavelength - kLambdaMin) / 5.0;
  int row = static_cast<int>(std::floor(u));
  assert(row >= 0 && row + 1 < 95);
  assert(
    CIE_2_DEG_COLOR_MATCHING_FUNCTIONS[4 * row] <= wavelength &&
    CIE_2_DEG_COLOR_MATCHING_FUNCTIONS[4 * (row + 1)] >= wavelength);
  u -= row;
  return CIE_2_DEG_COLOR_MATCHING_FUNCTIONS[4 * row + column] * (1.0 - u) +
         CIE_2_DEG_COLOR_MATCHING_FUNCTIONS[4 * (row + 1) + column] * u;
}

auto Interpolate(
  const std::vector<double> &wavelengths, const std::vector<double> &wavelength_function,
  double wavelength) -> double {
  assert(wavelength_function.size() == wavelengths.size());
  if(wavelength < wavelengths[0]) { return wavelength_function[0]; }
  for(unsigned int i = 0; i < wavelengths.size() - 1; ++i) {
    if(wavelength < wavelengths[i + 1]) {
      double u = (wavelength - wavelengths[i]) / (wavelengths[i + 1] - wavelengths[i]);
      return wavelength_function[i] * (1.0 - u) + wavelength_function[i + 1] * u;
    }
  }
  return wavelength_function[wavelength_function.size() - 1];
}

/*
<p>We can then implement a utility function to compute the "spectral radiance to
luminance" conversion constants (see Section 14.3 in <a
href="https://arxiv.org/pdf/1612.04336.pdf">A Qualitative and Quantitative
Evaluation of 8 Clear Sky Models</a> for their definitions):
*/

// The returned constants are in lumen.nm / watt.
void ComputeSpectralRadianceToLuminanceFactors(
  const std::vector<double> &wavelengths, const std::vector<double> &solar_irradiance,
  double lambda_power, double *k_r, double *k_g, double *k_b) {
  *k_r = 0.0;
  *k_g = 0.0;
  *k_b = 0.0;
  double solar_r = Interpolate(wavelengths, solar_irradiance, kLambdaR);
  double solar_g = Interpolate(wavelengths, solar_irradiance, kLambdaG);
  double solar_b = Interpolate(wavelengths, solar_irradiance, kLambdaB);
  int dlambda = 1;
  for(int lambda = kLambdaMin; lambda < kLambdaMax; lambda += dlambda) {
    double x_bar = CieColorMatchingFunctionTableValue(lambda, 1);
    double y_bar = CieColorMatchingFunctionTableValue(lambda, 2);
    double z_bar = CieColorMatchingFunctionTableValue(lambda, 3);
    const double *xyz2srgb = XYZ_TO_SRGB;
    double r_bar = xyz2srgb[0] * x_bar + xyz2srgb[1] * y_bar + xyz2srgb[2] * z_bar;
    double g_bar = xyz2srgb[3] * x_bar + xyz2srgb[4] * y_bar + xyz2srgb[5] * z_bar;
    double b_bar = xyz2srgb[6] * x_bar + xyz2srgb[7] * y_bar + xyz2srgb[8] * z_bar;
    double irradiance = Interpolate(wavelengths, solar_irradiance, lambda);
    *k_r += r_bar * irradiance / solar_r * pow(lambda / kLambdaR, lambda_power);
    *k_g += g_bar * irradiance / solar_g * pow(lambda / kLambdaG, lambda_power);
    *k_b += b_bar * irradiance / solar_b * pow(lambda / kLambdaB, lambda_power);
  }
  *k_r *= MAX_LUMINOUS_EFFICACY * dlambda;
  *k_g *= MAX_LUMINOUS_EFFICACY * dlambda;
  *k_b *= MAX_LUMINOUS_EFFICACY * dlambda;
}

void ConvertSpectrumToLinearSrgb(
  const std::vector<double> &wavelengths, const std::vector<double> &spectrum, double *r,
  double *g, double *b) {
  double x = 0.0;
  double y = 0.0;
  double z = 0.0;
  const int dlambda = 1;
  for(int lambda = kLambdaMin; lambda < kLambdaMax; lambda += dlambda) {
    double value = Interpolate(wavelengths, spectrum, lambda);
    x += CieColorMatchingFunctionTableValue(lambda, 1) * value;
    y += CieColorMatchingFunctionTableValue(lambda, 2) * value;
    z += CieColorMatchingFunctionTableValue(lambda, 3) * value;
  }
  *r = MAX_LUMINOUS_EFFICACY *
       (XYZ_TO_SRGB[0] * x + XYZ_TO_SRGB[1] * y + XYZ_TO_SRGB[2] * z) * dlambda;
  *g = MAX_LUMINOUS_EFFICACY *
       (XYZ_TO_SRGB[3] * x + XYZ_TO_SRGB[4] * y + XYZ_TO_SRGB[5] * z) * dlambda;
  *b = MAX_LUMINOUS_EFFICACY *
       (XYZ_TO_SRGB[6] * x + XYZ_TO_SRGB[7] * y + XYZ_TO_SRGB[8] * z) * dlambda;
}

auto luminanceFromRadiance(double dlambda, const glm::vec3 &lambdas) -> glm::mat4 {
  auto coeff = [dlambda](double lambda, int component) {
    double x = CieColorMatchingFunctionTableValue(lambda, 1);
    double y = CieColorMatchingFunctionTableValue(lambda, 2);
    double z = CieColorMatchingFunctionTableValue(lambda, 3);
    return static_cast<float>(
      (XYZ_TO_SRGB[component * 3] * x + XYZ_TO_SRGB[component * 3 + 1] * y +
       XYZ_TO_SRGB[component * 3 + 2] * z) *
      dlambda);
  };
  glm::mat4 luminance_from_radiance{
    coeff(lambdas[0], 0),
    coeff(lambdas[1], 0),
    coeff(lambdas[2], 0),
    0.0,
    coeff(lambdas[0], 1),
    coeff(lambdas[1], 1),
    coeff(lambdas[2], 1),
    0.0,
    coeff(lambdas[0], 2),
    coeff(lambdas[1], 2),
    coeff(lambdas[2], 2),
    0.0,
    0.0,
    0.0,
    0.0,
    0.0};
  luminance_from_radiance = glm::transpose(luminance_from_radiance);
  return luminance_from_radiance;
}

auto makeTex(
  Device &device, vk::ImageType type, uint32_t width, uint32_t height, uint32_t depth,
  vk::Format format, const std::string &name) -> std::unique_ptr<Texture> {
  vk::SharingMode sharingMode = vk::SharingMode::eExclusive;
  uint32_t queueFamilyIndexCount = 0;
  uint32_t queueFamilyIndices[]{device.graphicsIndex(), device.computeIndex()};
  if(device.graphicsIndex() != device.computeIndex()) {
    sharingMode = vk::SharingMode::eConcurrent;
    queueFamilyIndexCount = 2;
  }

  auto texture = std::make_unique<Texture>(
    device,
    vk::ImageCreateInfo{
      {},
      type,
      format,
      {width, height, depth},
      1,
      1,
      vk::SampleCountFlagBits::e1,
      vk::ImageTiling::eOptimal,
      vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferSrc |
        vk::ImageUsageFlagBits::eStorage,
      sharingMode,
      queueFamilyIndexCount,
      queueFamilyIndices},
    VmaAllocationCreateInfo{{}, VMA_MEMORY_USAGE_GPU_ONLY});
  texture->setImageView(
    type == vk::ImageType::e2D ? vk::ImageViewType::e2D : vk::ImageViewType::e3D,
    vk::ImageAspectFlagBits::eColor);
  texture->setSampler(
    {{},
     vk::Filter::eLinear,
     vk::Filter::eLinear,
     vk::SamplerMipmapMode::eLinear,
     vk::SamplerAddressMode::eClampToEdge,
     vk::SamplerAddressMode::eClampToEdge,
     vk::SamplerAddressMode::eClampToEdge});
  return texture;
}
}

AtmosphereModel::AtmosphereModel(
  Device &device, double sun_angular_radius, double bottom_radius, double top_radius,
  float length_unit_in_meters, glm::vec3 sunDirection, glm::vec3 earthCenter)
  : device{device} {
  // Values from "Reference Solar Spectral Irradiance: ASTM G-173", ETR column
  // (see http://rredc.nrel.gov/solar/spectra/am1.5/ASTMG173/ASTMG173.html),
  // summed and averaged in each bin (e.g. the value for 360nm is the average
  // of the ASTM G-173 values for all wavelengths between 360 and 370nm).
  // Values in W.m^-2.
  constexpr int kLambdaMin = 360;
  constexpr int kLambdaMax = 830;
  constexpr double kSolarIrradiance[48] = {
    1.11776, 1.14259, 1.01249, 1.14716, 1.72765, 1.73054, 1.6887,  1.61253,
    1.91198, 2.03474, 2.02042, 2.02212, 1.93377, 1.95809, 1.91686, 1.8298,
    1.8685,  1.8931,  1.85149, 1.8504,  1.8341,  1.8345,  1.8147,  1.78158,
    1.7533,  1.6965,  1.68194, 1.64654, 1.6048,  1.52143, 1.55622, 1.5113,
    1.474,   1.4482,  1.41018, 1.36775, 1.34188, 1.31429, 1.28303, 1.26758,
    1.2367,  1.2082,  1.18737, 1.14683, 1.12362, 1.1058,  1.07124, 1.04992};
  // Values from http://www.iup.uni-bremen.de/gruppen/molspec/databases/
  // referencespectra/o3spectra2011/index.html for 233K, summed and averaged in
  // each bin (e.g. the value for 360nm is the average of the original values
  // for all wavelengths between 360 and 370nm). Values in m^2.
  constexpr double kOzoneCrossSection[48] = {
    1.18e-27,  2.182e-28, 2.818e-28, 6.636e-28, 1.527e-27, 2.763e-27, 5.52e-27,
    8.451e-27, 1.582e-26, 2.316e-26, 3.669e-26, 4.924e-26, 7.752e-26, 9.016e-26,
    1.48e-25,  1.602e-25, 2.139e-25, 2.755e-25, 3.091e-25, 3.5e-25,   4.266e-25,
    4.672e-25, 4.398e-25, 4.701e-25, 5.019e-25, 4.305e-25, 3.74e-25,  3.215e-25,
    2.662e-25, 2.238e-25, 1.852e-25, 1.473e-25, 1.209e-25, 9.423e-26, 7.455e-26,
    6.566e-26, 5.105e-26, 4.15e-26,  4.228e-26, 3.237e-26, 2.451e-26, 2.801e-26,
    2.534e-26, 1.624e-26, 1.465e-26, 2.078e-26, 1.383e-26, 7.105e-27};
  // From https://en.wikipedia.org/wiki/Dobson_unit, in molecules.m^-2.
  constexpr double kDobsonUnit = 2.687e20;
  // Maximum number density of ozone molecules, in m^-3 (computed so at to get
  // 300 Dobson units of ozone - for this we divide 300 DU by the integral of
  // the ozone density profile defined below, which is equal to 15km).
  constexpr double kMaxOzoneNumberDensity = 300.0 * kDobsonUnit / 15000.0;
  // Wavelength independent solar irradiance "spectrum" (not physically
  // realistic, but was used in the original implementation).
  constexpr double kRayleigh = 1.24062e-6;
  constexpr double kRayleighScaleHeight = 8000.0;
  constexpr double kMieScaleHeight = 1200.0;
  constexpr double kMieAngstromAlpha = 0.0;
  constexpr double kMieAngstromBeta = 5.328e-3;
  constexpr double kMieSingleScatteringAlbedo = 0.9;
  constexpr double kMiePhaseFunctionG = 0.8;
  constexpr double kGroundAlbedo = 0.1;
  const double max_sun_zenith_angle = 120.0 / 180.0 * glm::pi<double>();

  DensityProfileLayer rayleigh_layer(0.0, 1.0, -1.0 / kRayleighScaleHeight, 0.0, 0.0);
  DensityProfileLayer mie_layer(0.0, 1.0, -1.0 / kMieScaleHeight, 0.0, 0.0);
  // Density profile increasing linearly from 0 to 1 between 10 and 25km, and
  // decreasing linearly from 1 to 0 between 25 and 40km. This is an approximate
  // profile from http://www.kln.ac.lk/science/Chemistry/Teaching_Resources/
  // Documents/Introduction%20to%20atmospheric%20chemistry.pdf (page 10).
  std::vector<DensityProfileLayer> ozone_density;
  ozone_density.emplace_back(25000.0, 0.0, 0.0, 1.0 / 15000.0, -2.0 / 3.0);
  ozone_density.emplace_back(0.0, 0.0, 0.0, -1.0 / 15000.0, 8.0 / 3.0);

  std::vector<double> wavelengths;
  std::vector<double> solar_irradiance;
  std::vector<double> rayleigh_scattering;
  std::vector<double> mie_scattering;
  std::vector<double> mie_extinction;
  std::vector<double> absorption_extinction;
  std::vector<double> ground_albedo;
  for(int l = kLambdaMin; l <= kLambdaMax; l += 10) {
    double lambda = static_cast<double>(l) * 1e-3; // micro-meters
    double mie = kMieAngstromBeta / kMieScaleHeight * pow(lambda, -kMieAngstromAlpha);
    wavelengths.push_back(l);
    solar_irradiance.push_back(kSolarIrradiance[(l - kLambdaMin) / 10]);
    rayleigh_scattering.push_back(kRayleigh * pow(lambda, -4));
    mie_scattering.push_back(mie * kMieSingleScatteringAlbedo);
    mie_extinction.push_back(mie);
    absorption_extinction.push_back(
      kMaxOzoneNumberDensity * kOzoneCrossSection[(l - kLambdaMin) / 10]);
    ground_albedo.push_back(kGroundAlbedo);
  }
  std::vector<DensityProfileLayer> rayleigh_density{rayleigh_layer};
  std::vector<DensityProfileLayer> mie_density{mie_layer};
  initParameter(
    wavelengths, solar_irradiance, sun_angular_radius, bottom_radius, top_radius,
    rayleigh_density, rayleigh_scattering, mie_density, mie_scattering, mie_extinction,
    kMiePhaseFunctionG, ozone_density, absorption_extinction, ground_albedo,
    max_sun_zenith_angle, length_unit_in_meters, 15, 1e-5, -glm::normalize(sunDirection),
    earthCenter);
}

AtmosphereModel::AtmosphereModel(
  Device &device, const std::vector<double> &wavelengths,
  const std::vector<double> &solar_irradiance, double sun_angular_radius,
  double bottom_radius, double top_radius,
  const std::vector<DensityProfileLayer> &rayleigh_density,
  const std::vector<double> &rayleigh_scattering,
  const std::vector<DensityProfileLayer> &mie_density,
  const std::vector<double> &mie_scattering, const std::vector<double> &mie_extinction,
  double mie_phase_function_g, const std::vector<DensityProfileLayer> &absorption_density,
  const std::vector<double> &absorption_extinction,
  const std::vector<double> &ground_albedo, double max_sun_zenith_angle,
  float length_unit_in_meters, unsigned int num_precomputed_wavelengths,
  float exposure_scale, glm::vec3 sunDirection, glm::vec3 earthCenter)
  : device{device} {
  initParameter(
    wavelengths, solar_irradiance, sun_angular_radius, bottom_radius, top_radius,
    rayleigh_density, rayleigh_scattering, mie_density, mie_scattering, mie_extinction,
    mie_phase_function_g, absorption_density, absorption_extinction, ground_albedo,
    max_sun_zenith_angle, length_unit_in_meters, num_precomputed_wavelengths,
    exposure_scale, -glm::normalize(sunDirection), earthCenter);
}

auto AtmosphereModel::atmosphereUBO() const -> BufferInfo {
  return atmosphereUBO_->bufferInfo();
}
auto AtmosphereModel::sunUBO() const -> BufferInfo { return sunUBO_->bufferInfo(); }
auto AtmosphereModel::transmittanceTex() -> Texture & { return *transmittanceTex_; }
auto AtmosphereModel::scatteringTex() -> Texture & { return *scatteringTex_; }
auto AtmosphereModel::irradianceTex() -> Texture & { return *irradianceTex_; }

auto AtmosphereModel::updateSunIntesity(float intensity) -> void {
  sunUBO_->ptr<SunUniform>()->intensity = intensity;
}
auto AtmosphereModel::updateExpsure(float exposure) -> void {
  sunUBO_->ptr<SunUniform>()->exposure = exposure * exposure_scale;
}
auto AtmosphereModel::updateSunDirection(glm::vec3 sunDirection) -> void {
  sunUBO_->ptr<SunUniform>()->sun_direction = {-glm::normalize(sunDirection), 0};
}
auto AtmosphereModel::updateEarthCenter(glm::vec3 earthCenter) -> void {
  sunUBO_->ptr<SunUniform>()->earth_center = {
    earthCenter, bottom_radius / length_unit_in_meters};
}

auto AtmosphereModel::createDescriptors() -> void {
  auto vkDevice = device.vkDevice();
  transmittanceSetDef.init(vkDevice);
  transmittancePipeDef.set(transmittanceSetDef);
  transmittancePipeDef.init(vkDevice);

  singleScatteringSetDef.init(vkDevice);
  singleScatteringPipeDef.set(singleScatteringSetDef);
  singleScatteringPipeDef.init(vkDevice);

  scatteringDensitySetDef.init(vkDevice);
  scatteringDensityPipeDef.set(scatteringDensitySetDef);
  scatteringDensityPipeDef.init(vkDevice);

  multipleScatteringSetDef.init(vkDevice);
  multipleScatteringPipeDef.set(multipleScatteringSetDef);
  multipleScatteringPipeDef.init(vkDevice);

  indirectIrradianceSetDef.init(vkDevice);
  indirectIrradiancePipeDef.set(indirectIrradianceSetDef);
  indirectIrradiancePipeDef.init(vkDevice);

  directIrradianceSetDef.init(vkDevice);
  directIrradiancePipeDef.set(directIrradianceSetDef);
  directIrradiancePipeDef.init(vkDevice);

  descriptorPool = DescriptorPoolMaker()
                     .pipelineLayout(transmittancePipeDef)
                     .pipelineLayout(singleScatteringPipeDef)
                     .pipelineLayout(scatteringDensityPipeDef)
                     .pipelineLayout(multipleScatteringPipeDef)
                     .pipelineLayout(indirectIrradiancePipeDef)
                     .pipelineLayout(directIrradiancePipeDef)
                     .createUnique(vkDevice);

  createTransmittanceSets();
  createDirectIrradianceSets();
  createSingleScatteringSets();
  createScatteringDensitySets();
  createIndirectIrradianceSets();
  createMultipleScatteringSets();
}

auto AtmosphereModel::init(uint32_t num_scattering_orders) -> void {
  deltaIrradianceTex = makeTex(
    device, vk::ImageType::e2D, IRRADIANCE_TEXTURE_WIDTH, IRRADIANCE_TEXTURE_HEIGHT, 1,
    vk::Format::eR32G32B32A32Sfloat, "deltaIrradianceTex");
  deltaRayleighScatteringTex = makeTex(
    device, vk::ImageType::e3D, SCATTERING_TEXTURE_WIDTH, SCATTERING_TEXTURE_HEIGHT,
    SCATTERING_TEXTURE_DEPTH, vk::Format::eR32G32B32A32Sfloat,
    "deltaRayleighScatteringTex");
  deltaMieScatteringTex = makeTex(
    device, vk::ImageType::e3D, SCATTERING_TEXTURE_WIDTH, SCATTERING_TEXTURE_HEIGHT,
    SCATTERING_TEXTURE_DEPTH, vk::Format::eR32G32B32A32Sfloat, "deltaMieScatteringTex");
  deltaScatteringDensityTex = makeTex(
    device, vk::ImageType::e3D, SCATTERING_TEXTURE_WIDTH, SCATTERING_TEXTURE_HEIGHT,
    SCATTERING_TEXTURE_DEPTH, vk::Format::eR32G32B32A32Sfloat,
    "deltaScatteringDensityTex");

  createDescriptors();

  compute(num_scattering_orders);

  deltaIrradianceTex.reset();
  deltaRayleighScatteringTex.reset();
  deltaMieScatteringTex.reset();
  deltaScatteringDensityTex.reset();
}

auto AtmosphereModel::compute(uint32_t num_scattering_orders) -> void {
  constexpr double _kLambdaMin = 360.0;
  constexpr double _kLambdaMax = 830.0;
  int num_iterations = (int(num_precomputed_wavelengths_) + 2) / 3;
  double dlambda = (_kLambdaMax - _kLambdaMin) / (3 * num_iterations);

  for(int i = 0; i < num_iterations; ++i) {
    glm::vec3 lambdas{
      _kLambdaMin + (3 * i + 0.5) * dlambda, _kLambdaMin + (3 * i + 1.5) * dlambda,
      _kLambdaMin + (3 * i + 2.5) * dlambda};

    glm::mat4 luminance_from_radiance = luminanceFromRadiance(dlambda, lambdas);
    precompute(lambdas, luminance_from_radiance, i > 0, num_scattering_orders);
  }

  atmosphereUBO_->ptr<AtmosphereUniform>()->atmosphere =
    calcAtmosphereParams({kLambdaR, kLambdaG, kLambdaB});

  device.execSyncInComputeQueue(
    [&](vk::CommandBuffer cb) { recordTransmittanceCMD(cb); });
}

auto AtmosphereModel::precompute(
  const glm::vec3 &lambdas, const glm::mat4 &luminance_from_radiance, bool cumulate,
  uint32_t num_scattering_orders) -> void {
  auto _cumulate = vk::Bool32(cumulate);
  atmosphereUBO_->ptr<AtmosphereUniform>()->atmosphere = calcAtmosphereParams(lambdas);
  device.execSyncInComputeQueue(
    [&](vk::CommandBuffer cb) { recordTransmittanceCMD(cb); });
  device.execSyncInComputeQueue(
    [&](vk::CommandBuffer cb) { recordDirectIrradianceCMD(cb, _cumulate); });
  device.execSyncInComputeQueue([&](vk::CommandBuffer cb) {
    recordSingleScatteringCMD(cb, luminance_from_radiance, _cumulate);
  });

  for(auto scatteringOrder = 2u; scatteringOrder <= num_scattering_orders;
      ++scatteringOrder) {
    device.execSyncInComputeQueue(
      [&](vk::CommandBuffer cb) { recordScatteringDensityCMD(cb, scatteringOrder); });
    device.execSyncInComputeQueue([&](vk::CommandBuffer cb) {
      recordIndirectIrradianceCMD(cb, luminance_from_radiance, scatteringOrder - 1);
    });
    device.execSyncInComputeQueue([&](vk::CommandBuffer cb) {
      recordMultipleScatteringCMD(cb, luminance_from_radiance);
    });
  }
}
auto AtmosphereModel::initParameter(
  const std::vector<double> &wavelengths, const std::vector<double> &solar_irradiance,
  double sun_angular_radius, double bottom_radius_, double top_radius,
  const std::vector<DensityProfileLayer> &rayleigh_density,
  const std::vector<double> &rayleigh_scattering,
  const std::vector<DensityProfileLayer> &mie_density,
  const std::vector<double> &mie_scattering, const std::vector<double> &mie_extinction,
  double mie_phase_function_g, const std::vector<DensityProfileLayer> &absorption_density,
  const std::vector<double> &absorption_extinction,
  const std::vector<double> &ground_albedo, double max_sun_zenith_angle,
  float length_unit_in_meters_, unsigned int num_precomputed_wavelengths,
  float exposure_scale_, glm::vec3 sunDirection, glm::vec3 earthCenter) -> void {
  num_precomputed_wavelengths_ = num_precomputed_wavelengths;
  bottom_radius = bottom_radius_;
  length_unit_in_meters = length_unit_in_meters_;
  {
    {
      double sky_k_r, sky_k_g, sky_k_b;
      sky_k_r = sky_k_g = sky_k_b = MAX_LUMINOUS_EFFICACY;

      double sun_k_r, sun_k_g, sun_k_b;
      ComputeSpectralRadianceToLuminanceFactors(
        wavelengths, solar_irradiance, 0 /* lambda_power */, &sun_k_r, &sun_k_g,
        &sun_k_b);

      atmosphereUBO_ =
        buffer::hostUniformBuffer(device, sizeof(AtmosphereUniform), "atmosphereUBO");

      auto *ptr = atmosphereUBO_->ptr<AtmosphereUniform>();
      ptr->transmittance_texture_width = TRANSMITTANCE_TEXTURE_WIDTH;
      ptr->transmittance_texture_height = TRANSMITTANCE_TEXTURE_HEIGHT;
      ptr->scattering_texture_r_size = SCATTERING_TEXTURE_R_SIZE;
      ptr->scattering_texture_mu_size = SCATTERING_TEXTURE_MU_SIZE;
      ptr->scattering_texture_mu_s_size = SCATTERING_TEXTURE_MU_S_SIZE;
      ptr->scattering_texture_nu_size = SCATTERING_TEXTURE_NU_SIZE;
      ptr->irradiance_texture_width = IRRADIANCE_TEXTURE_WIDTH;
      ptr->irradiance_texture_height = IRRADIANCE_TEXTURE_HEIGHT;

      ptr->sky_spectral_radiance_to_luminance = {sky_k_r, sky_k_g, sky_k_b, 0.0};
      ptr->sun_spectral_radiance_to_luminance = {sun_k_r, sun_k_g, sun_k_b, 0.0};
    }

    {
      auto spectrum =
        [=](const std::vector<double> &v, const glm::vec3 &lambdas, double scale) {
          double r = Interpolate(wavelengths, v, lambdas[0]) * scale;
          double g = Interpolate(wavelengths, v, lambdas[1]) * scale;
          double b = Interpolate(wavelengths, v, lambdas[2]) * scale;
          return glm::vec3{r, g, b};
        };

      auto density_layer = [=](const DensityProfileLayer &layer) {
        return DensityProfileLayer{
          layer.width / length_unit_in_meters_, layer.exp_term,
          layer.exp_scale * length_unit_in_meters_,
          layer.linear_term * length_unit_in_meters_, layer.constant_term};
      };

      auto density_profile = [=](std::vector<DensityProfileLayer> layers) {
        constexpr int kLayerCount = 2;
        while(layers.size() < kLayerCount) {
          layers.insert(layers.begin(), DensityProfileLayer());
        }
        DensityProfile result{};
        for(int i = 0; i < kLayerCount; ++i) {
          result.layers[i] = density_layer(layers[i]);
        }
        return result;
      };

      calcAtmosphereParams = [=](const glm::vec3 &lambdas) {
        return AtmosphereParameters{
          spectrum(solar_irradiance, lambdas, 1.0),
          float(sun_angular_radius),
          spectrum(rayleigh_scattering, lambdas, length_unit_in_meters),
          float(bottom_radius_ / length_unit_in_meters),
          spectrum(mie_scattering, lambdas, length_unit_in_meters),
          float(top_radius / length_unit_in_meters),
          spectrum(mie_extinction, lambdas, length_unit_in_meters),
          float(mie_phase_function_g),
          spectrum(absorption_extinction, lambdas, length_unit_in_meters),
          float(std::cos(max_sun_zenith_angle)),
          spectrum(ground_albedo, lambdas, 1.0),
          0,
          density_profile(rayleigh_density),
          density_profile(mie_density),
          density_profile(absorption_density),
        };
      };
    }

    {
      double white_point_r = 1.0;
      double white_point_g = 1.0;
      double white_point_b = 1.0;
      if(do_white_balance_) {
        ConvertSpectrumToLinearSrgb(
          wavelengths, solar_irradiance, &white_point_r, &white_point_g, &white_point_b);
        double white_point = (white_point_r + white_point_g + white_point_b) / 3.0;
        white_point_r /= white_point;
        white_point_g /= white_point;
        white_point_b /= white_point;
      }

      sunUBO_ = buffer::hostUniformBuffer(device, sizeof(SunUniform), "sunUBO");
      auto ptr = sunUBO_->ptr<SunUniform>();

      ptr->white_point = {white_point_r, white_point_g, white_point_b, 0};
      updateEarthCenter(earthCenter);
      ptr->sun_size = {glm::tan(sun_angular_radius), glm::cos(sun_angular_radius)};
      ptr->exposure = exposure_ * exposure_scale_;
      this->exposure_scale = exposure_scale_;
      ptr->intensity = 1.0;
      ptr->sun_direction = {sunDirection, 0};
    }
    {
      transmittanceTex_ = makeTex(
        device, vk::ImageType::e2D, TRANSMITTANCE_TEXTURE_WIDTH,
        TRANSMITTANCE_TEXTURE_HEIGHT, 1, vk::Format::eR32G32B32A32Sfloat,
        "transmittanceTex");
      scatteringTex_ = makeTex(
        device, vk::ImageType::e3D, SCATTERING_TEXTURE_WIDTH, SCATTERING_TEXTURE_HEIGHT,
        SCATTERING_TEXTURE_DEPTH, vk::Format::eR32G32B32A32Sfloat, "scatteringTex");
      irradianceTex_ = makeTex(
        device, vk::ImageType::e2D, IRRADIANCE_TEXTURE_WIDTH, IRRADIANCE_TEXTURE_HEIGHT,
        1, vk::Format::eR32G32B32A32Sfloat, "irradianceTex");
    }
  }
}
}