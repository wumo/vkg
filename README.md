# vkg

Graphics Engine on Vulkan written in C/C++ 20

- Deferred Shading
- Cascaded Shadow Map
- Atmosphere Rendering
- Ray tracing on NVIDIA RTX graphics card
- glTF models.

![sample](doc/sample.gif)

## Build

Requirements:
* `python3` >= 3.7
* `git` >= 2.0.0
* `gcc` >= 10.0; `Visual Studio 2019`; `clang` >= 10.0
* `cmake` >= 3.12
* `conan` >= 1.28
* Graphics driver that supports `Vulkan 1.2.0`
* `RayTracing Feature` requires RTX 20 series graphics card.


**Ubuntu**:
```
$ sudo apt install -y git gcc-10 cmake make
# install conan
$ pip install conan
```

**Windows**:
Using [scoop](https://scoop.sh/) to install dependencies:
```
# install scoop
$ Set-ExecutionPolicy RemoteSigned -scope CurrentUser
$ iex (new-object net.webclient).downloadstring('https://get.scoop.sh')
# install dependencies
$ scoop install python git gcc cmake
# install conan
$ pip install conan
```


Add aditional conan repo:
```
conan remote add bincrafters https://api.bintray.com/conan/bincrafters/public-conan
conan remote add wumo https://api.bintray.com/conan/wumo/public

```

Build with cmake:
```
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
```