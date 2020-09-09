# vkg

[![Download](https://api.bintray.com/packages/wumo/public/vkg:wumo/images/download.svg)](https://bintray.com/wumo/public/vkg:wumo/_latestVersion)

Graphics Engine on Vulkan written in C/C++ 20

- Deferred Shading
- Cascaded Shadow Mapping [[1]](#1)[[2]](#2)[[3]](#3)
- Atmospheric Rendering [[4]](#4)
- Ray tracing on NVIDIA RTX graphics card
- Load glTF models.
- Update vertices/transforms/materials/lights per frame.

![sample](doc/sample.gif)

## Other Language Bindings
* Kotlin binding - [vkgKt](https://github.com/wumo/vkgKt)

## Usage

Requirements:
* `python3` >= 3.7
* `git` >= 2.0.0
* `gcc` >= 10.0; `Visual Studio 2019`; `clang` >= 10.0
* `cmake` >= 3.12
* `conan` >= 1.28
* Graphics driver that supports `Vulkan 1.2.0`(nvidia-450 or above)
* `RayTracing Feature` requires RTX 20 series graphics card.

1. Use [conan](https://conan.io/) as the package manager. Add aditional conan repo:

```
conan remote add bincrafters https://api.bintray.com/conan/bincrafters/public-conan
conan remote add wumo https://api.bintray.com/conan/wumo/public
```

2. Add conan dependency `vkg/0.0.1@wumo/stable`.

3. That is all! Build with cmake or any tool you like (and the conan supports).

#### Sample code:

```c++
#include "vkg/render/renderer.hpp"
#include "vkg/util/colors.hpp"
#include "vkg/render/util/panning_camera.hpp"
using namespace vkg;
auto main() -> int {
  // window and renderer setting
  WindowConfig windowConfig{.title = "Sample", .width = 1080, .height = 720};
  FeatureConfig featureConfig{.numFrames = 2, .rayTrace = true};
  Renderer app{windowConfig, featureConfig};

  // scene setting
  SceneConfig sceneConfig{
    .maxNumTransforms = 100'0000,
    .maxNumPrimitives = 100'0000,
    .maxNumMeshInstances = 100'0000,
  };
  auto &scene = app.addScene(sceneConfig);

  // atmosphere setting
  auto &sky = scene.atmosphere();
  sky.enable(true);
  sky.setSunIntensity(10);
  sky.setSunDirection({-1, -0.1, 0});

  // primitive
  auto primitives =
    scene.newPrimitives(PrimitiveBuilder().sphere({}, 1.f).newPrimitive());
  // material
  auto &mat = scene.material(scene.newMaterial(MaterialType::eBRDF));
  mat.setColorFactor({Green, 1.f}).setPbrFactor({0, 0.3, 0.4, 0});
  // primitive + material => mesh
  auto mesh = scene.newMesh(primitives[0], mat.id());
  // mesh => node
  auto node = scene.newNode(Transform{});
  scene.node(node).addMeshes({mesh});
  // node => model
  auto model = scene.newModel({node});
  // model => instance
  auto sphere = scene.newModelInstance(model);

  // scene camera setting
  auto &camera = scene.camera();
  camera.setLocation({-5.610391, 0.049703, 16.386591});
  camera.setDirection({5.610391, -0.049703, -16.386591});

  // using builtin panning camera to change view
  PanningCamera panningCamera{camera};
  // capture input
  auto &input = app.window().input();

  //render loop
  app.loop([&](uint32_t frameIdx, double elapsedMs) {
    // update camera from input
    panningCamera.update(input);

    // apply transform per frame
    auto &ins = scene.modelInstance(sphere);
    auto t = ins.transform();
    t.translation.x -= elapsedMs * 0.001;
    ins.setTransform(t);
  });

  return 0;
}
```



## Build

Requirements:
* `python3` >= 3.7
* `git` >= 2.0.0
* `gcc` >= 10.0; `Visual Studio 2019`; `clang` >= 10.0
* `cmake` >= 3.12
* `conan` >= 1.28
* Graphics driver that supports `Vulkan 1.2.0` (nvidia-450 or above)
* `RayTracing Feature` requires RTX 20 series graphics card.


**Ubuntu**:

Tested on Ubuntu 20.04:

```
$ sudo apt install -y git gcc-10 cmake ninja-build python3-pip
$ pip3 install conan
```

**Windows**:

Instal `Visual Studio 2019` or using [scoop](https://scoop.sh/) to install dependencies:

```
# install scoop
$ Set-ExecutionPolicy RemoteSigned -scope CurrentUser
$ iex (new-object net.webclient).downloadstring('https://get.scoop.sh')
# install dependencies
$ scoop install python git gcc cmake ninja
# install conan
$ pip install conan
```


**Add aditional conan repo**:

```
conan remote add bincrafters https://api.bintray.com/conan/bincrafters/public-conan
conan remote add wumo https://api.bintray.com/conan/wumo/public
```

**Build with cmake**:

Runs in a proper terminal (Visual Studio Developer Command Prompt on Windows)
```
mkdir build && cd build
cmake ..  -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
```

## TODO
* [x] Ray Cone texture lod [[5]](#1)
* [ ] Multiple Scene/View
* [ ] Font Rendering
* [ ] ImGUI integration
* [ ] Custom ComputeShader/IntersectionShader Primitive
* [ ] Occlusion Culling
* [ ] Clear Coat 
* [ ] FrameGraph
* [ ] Terrain Rendering
* [ ] Ocean Rendering
* [ ] Cloud Rendering

## References

<a id="1">[1]</a>  [Cascaded Shadow Maps by lwjglgamdev](https://ahbejarano.gitbook.io/lwjglgamedev/chapter26)

<a id="2">[2]</a>  [Cascaded Shadow Maps by Microsoft](https://docs.microsoft.com/en-us/windows/win32/dxtecharts/cascaded-shadow-maps)

<a id="3">[3]</a>  [GPU Gems 3](https://developer.nvidia.com/gpugems/gpugems3/part-ii-light-and-shadows/chapter-10-parallel-split-shadow-maps-programmable-gpus)

<a id="4">[4]</a>  [Precomputed Atmospheric Scattering](https://github.com/ebruneton/precomputed_atmospheric_scattering)

<a id="5">[5]</a>  [Texture Level of Detail Strategies for Real-Time Ray Tracing](https://media.contentapi.ea.com/content/dam/ea/seed/presentations/2019-ray-tracing-gems-chapter-20-akenine-moller-et-al.pdf)