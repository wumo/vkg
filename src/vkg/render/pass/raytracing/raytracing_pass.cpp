#include "raytracing_pass.hpp"
#include "raytracing/comp/tlas_comp.hpp"
namespace vkg {
struct CompTLASPassIn {
  FrameGraphResource<BufferInfo> meshInstances;
  FrameGraphResource<uint32_t> meshInstancesCount;
  FrameGraphResource<SceneConfig> sceneConfig;
  FrameGraphResource<BufferInfo> primitives;
  FrameGraphResource<BufferInfo> matrices;
  FrameGraphResource<std::span<uint32_t>> countPerDrawGroup;
};

struct CompTLASPassOut {
  FrameGraphResource<uint32_t> tlasCount;
  FrameGraphResource<BufferInfo> tlas;
};

class CompTLASPass: public Pass<CompTLASPassIn, CompTLASPassOut> {
public:
  void setup(PassBuilder &builder) override {
    builder.read(passIn);
    passOut = {
      .tlasCount = builder.create<uint32_t>("tlasInstanceCount"),
      .tlas = builder.create<BufferInfo>("tlasInstances"),
    };
  }
  void compile(RenderContext &ctx, Resources &resources) override {
    if(!init) {
      init = true;

      setDef.init(ctx.device);
      pipeDef.set(setDef);
      pipeDef.init(ctx.device);
      pipe = ComputePipelineMaker(ctx.device)
               .layout(pipeDef.layout())
               .shader(Shader{shader::raytracing::comp::tlas_comp_span, local_size, 1, 1})
               .createUnique();

      descriptorPool = DescriptorPoolMaker()
                         .pipelineLayout(pipeDef, ctx.numFrames)
                         .createUnique(ctx.device);

      auto sceneConfig = resources.get(passIn.sceneConfig);

      frames.resize(ctx.numFrames);
      for(int i = 0; i < ctx.numFrames; ++i) {
        auto &frame = frames[i];
        frame.set = setDef.createSet(*descriptorPool);

        frame.tlasInstanceCount = buffer::devStorageBuffer(
          resources.device, sizeof(uint32_t), toString(name, "_tlasInstanceCount_", i));
        frame.tlasInstances = buffer::devStorageBuffer(
          resources.device,
          sizeof(VkAccelerationStructureInstanceKHR) * sceneConfig.maxNumMeshInstances,
          toString(name, "_tlasInstances_", i));
      }
    }
    auto &frame = frames[ctx.frameIndex];

    setDef.meshInstances(resources.get(passIn.meshInstances));
    setDef.primitives(resources.get(passIn.primitives));
    setDef.matrices(resources.get(passIn.matrices));
    setDef.tlasInstanceCount(frame.tlasInstanceCount->bufferInfo());
    setDef.tlasInstances(frame.tlasInstances->bufferInfo());
    setDef.update(frame.set);

    auto countPerDrawGroup = resources.get(passIn.countPerDrawGroup);
    auto tlasCount = countPerDrawGroup[value(DrawGroup::BRDF)] +
                     countPerDrawGroup[value(DrawGroup::Reflective)] +
                     countPerDrawGroup[value(DrawGroup::Refractive)];
    resources.set(passOut.tlasCount, tlasCount);
    resources.set(passOut.tlas, frame.tlasInstances->bufferInfo());
  }
  void execute(RenderContext &ctx, Resources &resources) override {
    auto meshInstancesCount = resources.get(passIn.meshInstancesCount);
    if(meshInstancesCount == 0) return;

    auto &frame = frames[ctx.frameIndex];

    auto cb = ctx.cb;

    ctx.device.begin(cb, name + " compute tlas");

    auto bufInfo = frame.tlasInstanceCount->bufferInfo();
    cb.fillBuffer(bufInfo.buffer, bufInfo.offset, sizeof(uint32_t), 0u);
    cb.pipelineBarrier(
      vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eComputeShader, {},
      vk::MemoryBarrier{
        vk::AccessFlagBits::eTransferWrite, vk::AccessFlagBits::eShaderRead},
      nullptr, nullptr);

    auto totalDispatch = meshInstancesCount;
    auto maxCG = ctx.device.limits().maxComputeWorkGroupCount;
    auto totalGroup = uint32_t(std::ceil(totalDispatch / double(local_size)));
    auto dx = std::min(totalGroup, maxCG[0]);
    totalGroup = uint32_t(totalGroup / double(dx));
    auto dy = std::min(std::max(totalGroup, 1u), maxCG[1]);
    totalGroup = uint32_t(totalGroup / double(dy));
    auto dz = std::min(std::max(totalGroup, 1u), maxCG[2]);

    cb.bindPipeline(vk::PipelineBindPoint::eCompute, *pipe);
    cb.bindDescriptorSets(
      vk::PipelineBindPoint::eCompute, pipeDef.layout(), pipeDef.set.set(), frame.set,
      nullptr);
    pushConstant = {
      .totalMeshInstances = meshInstancesCount,
      .frame = ctx.frameIndex,
    };
    cb.pushConstants<PushConstant>(
      pipeDef.layout(), vk::ShaderStageFlagBits::eCompute, 0, pushConstant);
    cb.dispatch(dx, dy, dz);

    cb.pipelineBarrier(
      vk::PipelineStageFlagBits::eComputeShader,
      vk::PipelineStageFlagBits::eAccelerationStructureBuildNV, {},
      vk::MemoryBarrier{
        vk::AccessFlagBits::eShaderWrite,
        vk::AccessFlagBits::eAccelerationStructureWriteNV},
      nullptr, nullptr);
    ctx.device.end(cb);
  }

private:
  struct ComputeTLASSetDef: DescriptorSetDef {
    __buffer__(meshInstances, vkStage::eCompute);
    __buffer__(primitives, vkStage::eCompute);
    __buffer__(matrices, vkStage::eCompute);
    __buffer__(tlasInstanceCount, vkStage::eCompute);
    __buffer__(tlasInstances, vkStage::eCompute);
  } setDef;
  struct PushConstant {
    uint32_t totalMeshInstances;
    uint32_t frame;
  } pushConstant{};
  struct RTPipeDef: PipelineLayoutDef {
    __push_constant__(constant, vkStage::eCompute, PushConstant);
    __set__(set, ComputeTLASSetDef);
  } pipeDef;

  struct FrameResource {
    vk::DescriptorSet set;

    std::unique_ptr<Buffer> tlasInstanceCount;
    std::unique_ptr<Buffer> tlasInstances;
  };
  std::vector<FrameResource> frames;

  vk::UniqueDescriptorPool descriptorPool;
  vk::UniquePipeline pipe;
  const uint32_t local_size = 64;

  bool init{false};
};

void RayTracingPass::setup(PassBuilder &builder) {}
void RayTracingPass::compile(RenderContext &ctx, Resources &resources) {}
void RayTracingPass::execute(RenderContext &ctx, Resources &resources) {}

}