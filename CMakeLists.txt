cmake_minimum_required(VERSION 3.12)
project(vkg LANGUAGES C CXX)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

include(${CMAKE_CURRENT_LIST_DIR}/cmake/symlink.cmake)
symlink(assets bin/assets)

include(${CMAKE_CURRENT_LIST_DIR}/cmake/conan.cmake)
conan_cmake_run(BASIC_SETUP CONANFILE conanfile.py BUILD missing)

option(BUILD_TEST "Build test" ON)
option(BUILD_SHARED "Build shared lib" ON)

set(sources
  src/vkg/base/impl/vma.cpp
  src/vkg/base/impl/stb.cpp
  src/vkg/base/impl/tinygltf.cpp
  
  src/vkg/util/syntactic_sugar.cpp
  src/vkg/util/fps_meter.cpp
  
  src/vkg/base/window.cpp
  src/vkg/base/instance.cpp
  src/vkg/base/device.cpp
  src/vkg/base/device_marker.cpp
  src/vkg/base/swapchain.cpp
  src/vkg/base/base.cpp
  
  src/vkg/base/resource/buffer.cpp
  src/vkg/base/resource/buffers.cpp
  src/vkg/base/resource/texture.cpp
  src/vkg/base/resource/texture_layout.cpp
  src/vkg/base/resource/texture_creator.cpp
  src/vkg/base/resource/texture_copy.cpp
  src/vkg/base/resource/texture_upload.cpp
  src/vkg/base/resource/texture_mipmap.cpp
  
  src/vkg/base/pipeline/pipeline_query.cpp
  src/vkg/base/pipeline/render_pass.cpp
  src/vkg/base/pipeline/shaders.cpp
  src/vkg/base/pipeline/descriptors.cpp
  src/vkg/base/pipeline/descriptor_updaters.cpp
  src/vkg/base/pipeline/descriptor_def.cpp
  src/vkg/base/pipeline/descriptor_pool.cpp
  src/vkg/base/pipeline/render_pass.cpp
  src/vkg/base/pipeline/pipeline.cpp
  src/vkg/base/pipeline/pipeline_def.cpp
  src/vkg/base/pipeline/pipeline_macro.cpp
  src/vkg/base/pipeline/graphics_pipeline.cpp
  src/vkg/base/pipeline/compute_pipeline_maker.cpp
  src/vkg/base/pipeline/raytracing_pipeline.cpp
  
  src/vkg/math/matrices.cpp
  src/vkg/math/frustum.cpp
  
  src/vkg/render/model/aabb.cpp
  src/vkg/render/model/transform.cpp
  src/vkg/render/model/animation.cpp
  src/vkg/render/model/light.cpp
  src/vkg/render/model/material.cpp
  src/vkg/render/model/mesh.cpp
  src/vkg/render/model/model.cpp
  src/vkg/render/model/model_instance.cpp
  src/vkg/render/model/node.cpp
  src/vkg/render/model/primitive.cpp
  src/vkg/render/model/camera.cpp
  
  src/vkg/render/builder/primitive_builder.cpp
  src/vkg/render/builder/gltf_loader.cpp
  
  src/vkg/render/util/panning_camera.cpp
  
  src/vkg/render/graph/frame_graph.cpp
  
  src/vkg/render/renderer.cpp
  src/vkg/render/scene.cpp
  src/vkg/render/scene_config.hpp
  src/vkg/render/scene_frame.cpp
  src/vkg/render/pass/compute_transf.cpp
  src/vkg/render/pass/compute_cull_drawcmd.cpp
  src/vkg/render/pass/deferred.cpp
  src/vkg/render/pass/deferred_execute.cpp
  src/vkg/render/pass/deferred_gbuffer.cpp
  src/vkg/render/pass/deferred_unlit.cpp
  src/vkg/render/pass/deferred_lighting.cpp
  src/vkg/render/pass/deferred_transparent.cpp
  src/vkg/render/pass/atmosphere_pass.cpp
  src/vkg/render/model/atmosphere.cpp)

set(definitions "")
if(WIN32)
  list(APPEND definitions "VK_USE_PLATFORM_WIN32_KHR")
endif(WIN32)
if(UNIX)
  list(APPEND definitions "VK_USE_PLATFORM_XCB_KHR" "VK_USE_PLATFORM_XLIB_KHR")
endif(UNIX)
list(APPEND definitions "USE_GLFW_WINDOW" "GLFW_INCLUDE_NONE")
list(APPEND definitions "USE_VALIDATION_LAYER")
list(APPEND sources src/vkg/base/window/platform/glfw_window.cpp)

if(BUILD_SHARED)
  add_library(vkg SHARED ${sources})
else()
  add_library(vkg STATIC ${sources})
endif()

static_shader_ns(vkg shader ${CMAKE_CURRENT_LIST_DIR}/assets/shaders
  ${CMAKE_CURRENT_BINARY_DIR}/shaders)

target_include_directories(vkg PUBLIC
  ${CMAKE_CURRENT_LIST_DIR}/src)
target_compile_definitions(vkg PUBLIC
  ${definitions}
  VULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1
  $<$<CONFIG:DEBUG>:DEBUG>)
target_link_libraries(vkg
  PUBLIC
  ${CONAN_LIBS}
  $<$<PLATFORM_ID:Linux>:dl>
  $<$<CXX_COMPILER_ID:GNU>:-static-libstdc++>
  )
install(TARGETS vkg)

if(BUILD_TEST)
  file(GLOB_RECURSE test-sources CONFIGURE_DEPENDS "test/*.cpp")
  foreach(file ${test-sources})
    get_filename_component(comp ${file} NAME_WE)
    add_executable(${comp} ${file})
    target_include_directories(${comp} PUBLIC
      ${CMAKE_CURRENT_LIST_DIR}/test)
    target_link_libraries(${comp} PRIVATE vkg)
  endforeach()
endif()
