#pragma once
#include "vkg/base/base.hpp"
#include "vkg/render/scene_config.hpp"
#include "vkg/render/graph/frame_graph.hpp"
#include "vkg/render/model/camera.hpp"
#include "vkg/math/frustum.hpp"

namespace vkg {
struct CamFrustumPassIn {
    FrameGraphResource<Camera *> camera;
};
struct CamFrustumPassOut {
    FrameGraphResource<std::span<Frustum>> camFrustum;
    FrameGraphResource<BufferInfo> camBuffer;
};

class CamFrustumPass: public Pass<CamFrustumPassIn, CamFrustumPassOut> {
public:
    void setup(PassBuilder &builder) override;
    void compile(RenderContext &ctx, Resources &resources) override;
    void execute(RenderContext &ctx, Resources &resources) override;

private:
    std::vector<Frustum> frustums;
    std::vector<std::unique_ptr<Buffer>> camBuffers;
    bool init{false};
};
}