#include "atmosphere_pass.hpp"
namespace vkg {

void AtmospherePass::setup(PassBuilder &builder) {
    builder.read(passIn.atmosphere);
    passOut = {
        .version = builder.create<uint64_t>("atmosphereVersion"),
        .atmosphere = builder.create<BufferInfo>("atmosphere"),
        .sun = builder.create<BufferInfo>("sun"),
        .transmittance = builder.create<Texture *>("transmittance"),
        .scattering = builder.create<Texture *>("scattering"),
        .irradiance = builder.create<Texture *>("irradiance"),
    };
}
void AtmospherePass::compile(RenderContext &ctx, Resources &resources) {
    auto atmosSetting = resources.get(passIn.atmosphere);

    if(!atmosSetting.isEnabled()) return;

    if(model_ == nullptr) {
        model_ = std::make_unique<AtmosphereModel>(
            ctx.device, atmosSetting.kSunAngularRadius_, atmosSetting.kBottomRadius_, atmosSetting.kTopRadius,
            atmosSetting.kLengthUnitInMeters_, atmosSetting.sunDirection(), atmosSetting.earthCenter());

        auto tStart = std::chrono::high_resolution_clock::now();

        model_->init(ctx.frameIndex);

        auto tEnd = std::chrono::high_resolution_clock::now();
        auto tDiff = std::chrono::duration<double, std::milli>(tEnd - tStart).count();
        debugLog("Generating sky map took ", tDiff, " ms");

        version++;
        resources.set(passOut.version, version);
        resources.set(passOut.atmosphere, model_->atmosphereUBO());
        resources.set(passOut.sun, model_->sunUBO());
        resources.set(passOut.transmittance, &model_->transmittanceTex());
        resources.set(passOut.scattering, &model_->scatteringTex());
        resources.set(passOut.irradiance, &model_->irradianceTex());
    }

    model_->updateSunIntesity(atmosSetting.sunIntensity());
    model_->updateExpsure(atmosSetting.exposure());
    model_->updateSunDirection(atmosSetting.sunDirection());
    model_->updateEarthCenter(atmosSetting.earthCenter());
}
}