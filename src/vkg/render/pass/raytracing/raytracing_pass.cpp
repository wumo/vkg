#include "raytracing_pass.hpp"
#include "raytracing/ray_rgen.hpp"
#include "raytracing/ray_rmiss.hpp"
#include "raytracing/shadow_ray_rmiss.hpp"
#include "raytracing/hit/unlit_rchit.hpp"
#include "raytracing/hit/brdf_rchit.hpp"
#include "raytracing/hit/reflective_rchit.hpp"
#include "raytracing/hit/refractive_rchit.hpp"
#include "raytracing/hit/brdf_atmos_rchit.hpp"
#include "raytracing/hit/reflective_atmos_rchit.hpp"
#include "raytracing/hit/refractive_atmos_rchit.hpp"
#include "raytracing/ray_atmos_rmiss.hpp"

namespace vkg {

struct CamPassIn {
  FrameGraphResource<Camera *> camera;
};
struct CamPassOut {
  FrameGraphResource<BufferInfo> camBuffer;
};

class CamPass: public Pass<CamPassIn, CamPassOut> {
public:
  void setup(PassBuilder &builder) override {
    builder.read(passIn.camera);
    passOut = {
      .camBuffer = builder.create<BufferInfo>("camBuffer"),
    };
  }
  void compile(RenderContext &ctx, Resources &resources) override {
    if(!init) {
      init = true;
      camBuffers.resize(ctx.numFrames);
      for(int i = 0; i < ctx.numFrames; ++i) {
        camBuffers[i] = buffer::devStorageBuffer(
          resources.device, sizeof(Camera::Desc) * ctx.numFrames,
          toString("camBuffer_", i));
      }
    }
    resources.set(passOut.camBuffer, camBuffers[ctx.frameIndex]->bufferInfo());
  }

  void execute(RenderContext &ctx, Resources &resources) override {
    auto *camera = resources.get(passIn.camera);
    auto desc = camera->desc();
    desc.frame = ctx.frameIndex;
    auto bufInfo = camBuffers[ctx.frameIndex]->bufferInfo();

    auto cb = ctx.cb;
    ctx.device.begin(cb, "update camera");
    cb.updateBuffer(bufInfo.buffer, bufInfo.offset, sizeof(desc), &desc);
    cb.pipelineBarrier(
      vk::PipelineStageFlagBits::eTransfer,
      vk::PipelineStageFlagBits::eRayTracingShaderNV, {},
      vk::MemoryBarrier{
        vk::AccessFlagBits::eTransferWrite, vk::AccessFlagBits::eShaderRead},
      nullptr, nullptr);
    ctx.device.end(cb);
  }

private:
  std::vector<std::unique_ptr<Buffer>> camBuffers;
  bool init{false};
};

void RayTracingPass::setup(PassBuilder &builder) {
  auto &compTLAS = builder.newPass<CompTLASPass>(
    "ComputeTLAS", {passIn.meshInstances, passIn.meshInstancesCount, passIn.sceneConfig,
                    passIn.primitives, passIn.matrices, passIn.countPerDrawGroup});
  compTlasPassOut = compTLAS.out();
  auto &cam = builder.newPass<CamPass>("CamPass", {passIn.camera});
  camBuffer = cam.out().camBuffer;

  builder.read(passIn.sceneConfig);
  builder.read(compTlasPassOut);
  builder.read(camBuffer);
  builder.read(passIn.meshInstances);
  builder.read(passIn.primitives);
  builder.read(passIn.positions);
  builder.read(passIn.normals);
  builder.read(passIn.uvs);
  builder.read(passIn.indices);
  builder.read(passIn.materials);
  builder.read(passIn.samplers);
  builder.read(passIn.lighting);
  builder.read(passIn.lights);
  builder.read(passIn.atmosSetting);
  builder.read(passIn.atmosphere);

  passOut = {
    .backImg = builder.write(passIn.backImg),
  };
}
void RayTracingPass::compile(RenderContext &ctx, Resources &resources) {
  auto *backImg = resources.get(passIn.backImg);
  auto samplers = resources.get(passIn.samplers);
  auto numValidSampler = resources.get(passIn.numValidSampler);

  if(!init) {
    init = true;

    auto sceneConfig = resources.get(passIn.sceneConfig);
    rtSetDef.textures.descriptorCount() = sceneConfig.maxNumTextures;
    rtSetDef.init(ctx.device);
    atmosphereSetDef.init(ctx.device);
    pipeDef.rt(rtSetDef);
    pipeDef.atmosphere(atmosphereSetDef);
    pipeDef.init(ctx.device);

    {
      using namespace shader::raytracing;
      {
        RayTracingPipelineMaker maker{ctx.device};
        maker.maxRecursionDepth(2)
          .rayGenGroup(Shader{ray_rgen_span, sceneConfig.maxNumTextures})
          .missGroup(Shader{ray_rmiss_span, sceneConfig.maxNumTextures})
          .missGroup(Shader{shadow_ray_rmiss_span, sceneConfig.maxNumTextures})
          .hitGroup(
            {.closestHit = Shader{hit::unlit_rchit_span, sceneConfig.maxNumTextures}})
          .hitGroup(
            {.closestHit = Shader{hit::brdf_rchit_span, sceneConfig.maxNumTextures}})
          .hitGroup(
            {.closestHit =
               Shader{hit::reflective_rchit_span, sceneConfig.maxNumTextures}})
          .hitGroup(
            {.closestHit =
               Shader{hit::refractive_rchit_span, sceneConfig.maxNumTextures}});
        std::tie(pipe, sbt) = maker.createUnique(pipeDef.layout(), nullptr);
      }
      {
        RayTracingPipelineMaker maker{ctx.device};
        maker.maxRecursionDepth(2)
          .rayGenGroup(Shader{ray_rgen_span, sceneConfig.maxNumTextures})
          .missGroup(Shader{ray_atmos_rmiss_span, sceneConfig.maxNumTextures})
          .missGroup(Shader{shadow_ray_rmiss_span, sceneConfig.maxNumTextures})
          .hitGroup(
            {.closestHit = Shader{hit::unlit_rchit_span, sceneConfig.maxNumTextures}})
          .hitGroup(
            {.closestHit =
               Shader{hit::brdf_atmos_rchit_span, sceneConfig.maxNumTextures}})
          .hitGroup(
            {.closestHit =
               Shader{hit::reflective_atmos_rchit_span, sceneConfig.maxNumTextures}})
          .hitGroup(
            {.closestHit =
               Shader{hit::refractive_atmos_rchit_span, sceneConfig.maxNumTextures}});
        std::tie(atmosPipe, atmosSbt) = maker.createUnique(pipeDef.layout(), nullptr);
      }
    }

    descriptorPool = DescriptorPoolMaker()
                       .pipelineLayout(pipeDef, ctx.numFrames)
                       .createUnique(resources.device);

    frames.resize(ctx.numFrames);
    for(auto &frame: frames) {
      frame.rtSet = rtSetDef.createSet(*descriptorPool);
      frame.atmosphereSet = atmosphereSetDef.createSet(*descriptorPool);
      rtSetDef.textures(0, uint32_t(samplers.size()), samplers.data());
      rtSetDef.update(frame.rtSet);
    }
  }
  auto &frame = frames[ctx.frameIndex];

  if(backImg != frame.backImg) {
    frame.backImg = backImg;
    auto w = frame.backImg->extent().width;
    auto h = frame.backImg->extent().height;
    using vkUsage = vk::ImageUsageFlagBits;
    frame.depthImg = image::make2DTex(
      toString("depthImg", ctx.frameIndex), ctx.device, w, h,
      vkUsage::eSampled | vkUsage::eStorage | vkUsage::eTransferSrc,
      vk::Format::eR32Sfloat);

    frame.depthImg->setSampler({});
  }

  if(numValidSampler > frame.lastNumValidSampler) {
    rtSetDef.textures(
      frame.lastNumValidSampler, numValidSampler - frame.lastNumValidSampler,
      samplers.data() + frame.lastNumValidSampler);
    frame.lastNumValidSampler = numValidSampler;
  }
  auto tlasCount = resources.get(compTlasPassOut.tlasCount);
  if(!frame.tlas.as || tlasCount > frame.tlas.geometryCount) {
    frame.tlas.type = vk::AccelerationStructureTypeNV::eTopLevel;
    frame.tlas.flags = vk::BuildAccelerationStructureFlagBitsNV::ePreferFastTrace;
    frame.tlas.geometryCount = tlasCount;
    allocAS(ctx.device, frame.tlas, tlasCount, 0, nullptr);
  }

  rtSetDef.as(*frame.tlas.as);
  rtSetDef.hdr(frame.backImg->imageView());
  rtSetDef.depth(frame.depthImg->imageView());
  rtSetDef.camera(resources.get(camBuffer));
  rtSetDef.meshInstances(resources.get(passIn.meshInstances));
  rtSetDef.primitives(resources.get(passIn.primitives));
  rtSetDef.positions(resources.get(passIn.positions));
  rtSetDef.normals(resources.get(passIn.normals));
  rtSetDef.uvs(resources.get(passIn.uvs));
  rtSetDef.indices(resources.get(passIn.indices));
  rtSetDef.materials(resources.get(passIn.materials));
  rtSetDef.lighting(resources.get(passIn.lighting));
  rtSetDef.lights(resources.get(passIn.lights));
  rtSetDef.update(frame.rtSet);

  if(auto atmosSetting = resources.get(passIn.atmosSetting); atmosSetting.isEnabled()) {
    atmosphereSetDef.atmosphere(resources.get(passIn.atmosphere.atmosphere));
    atmosphereSetDef.sun(resources.get(passIn.atmosphere.sun));
    atmosphereSetDef.transmittance(*resources.get(passIn.atmosphere.transmittance));
    atmosphereSetDef.scattering(*resources.get(passIn.atmosphere.scattering));
    atmosphereSetDef.irradiance(*resources.get(passIn.atmosphere.irradiance));
    atmosphereSetDef.update(frame.atmosphereSet);
  }
}

void RayTracingPass::execute(RenderContext &ctx, Resources &resources) {
  auto tlasCount = resources.get(compTlasPassOut.tlasCount);
  auto tlasInstances = resources.get(compTlasPassOut.tlas);
  auto atmosSetting = resources.get(passIn.atmosSetting);

  auto &frame = frames[ctx.frameIndex];

  auto cb = ctx.cb;
  ctx.device.begin(cb, "build tlas");

  vk::AccelerationStructureInfoNV info{
    frame.tlas.type, frame.tlas.flags, tlasCount, 0, nullptr};
  if(
    !frame.tlas.scratchBuffer || buildScratchBufferSize(ctx.device, *frame.tlas.as) >
                                   frame.tlas.scratchBuffer->bufferInfo().size)
    frame.tlas.scratchBuffer =
      allocBuildScratchBuffer(ctx.device, *frame.tlas.as, "tlasScratchBuffer");
  auto scratchBufInfo = frame.tlas.scratchBuffer->bufferInfo();
  cb.buildAccelerationStructureNV(
    info, tlasInstances.buffer, tlasInstances.offset, false, *frame.tlas.as, nullptr,
    scratchBufInfo.buffer, scratchBufInfo.offset);

  cb.pipelineBarrier(
    vk::PipelineStageFlagBits::eAccelerationStructureBuildNV,
    vk::PipelineStageFlagBits::eRayTracingShaderNV, {},
    vk::MemoryBarrier{
      vk::AccessFlagBits::eAccelerationStructureWriteNV,
      vk::AccessFlagBits::eAccelerationStructureReadNV},
    nullptr, nullptr);
  ctx.device.end(cb);

  ctx.device.begin(cb, "trace rays");

  image::transitTo(
    cb, *frame.backImg, vk::ImageLayout::eGeneral, vk::AccessFlagBits::eShaderWrite,
    vk::PipelineStageFlagBits::eRayTracingShaderNV);
  image::transitTo(
    cb, *frame.depthImg, vk::ImageLayout::eGeneral, vk::AccessFlagBits::eShaderWrite,
    vk::PipelineStageFlagBits::eRayTracingShaderNV);

  cb.bindDescriptorSets(
    vk::PipelineBindPoint::eRayTracingNV, pipeDef.layout(), pipeDef.rt.set(), frame.rtSet,
    nullptr);
  if(atmosSetting.isEnabled()) {
    cb.bindDescriptorSets(
      vk::PipelineBindPoint::eRayTracingNV, pipeDef.layout(), pipeDef.atmosphere.set(),
      frame.atmosphereSet, nullptr);
    cb.bindPipeline(vk::PipelineBindPoint::eRayTracingNV, *atmosPipe);
  } else
    cb.bindPipeline(vk::PipelineBindPoint::eRayTracingNV, *pipe);
  pushConstant = {
    .maxDepth = 2,
    .nbSamples = 1,
    .frame = ctx.frameIndex,
  };
  cb.pushConstants<PushConstant>(
    pipeDef.layout(),
    vk::ShaderStageFlagBits::eRaygenNV | vk::ShaderStageFlagBits::eClosestHitNV |
      vk::ShaderStageFlagBits::eMissNV | vk::ShaderStageFlagBits::eAnyHitNV,
    0, pushConstant);

  auto *_sbt = atmosSetting.isEnabled() ? &atmosSbt : &sbt;
  auto sbtBuffer = _sbt->shaderBindingTable->bufferInfo().buffer;
  cb.traceRaysNV(
    sbtBuffer, _sbt->rayGenOffset, sbtBuffer, _sbt->missGroupOffset,
    _sbt->missGroupStride, sbtBuffer, _sbt->hitGroupOffset, _sbt->hitGroupStride, nullptr,
    0, 0, frame.backImg->extent().width, frame.backImg->extent().height, 1);

  ctx.device.end(cb);
}

}