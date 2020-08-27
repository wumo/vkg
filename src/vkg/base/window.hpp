#pragma once

#include "vk_headers.hpp"

#if defined(USE_GLFW_WINDOW)
  #if !defined(GLFW_INCLUDE_NONE)
    #define GLFW_INCLUDE_NONE
  #endif
  #include <GLFW/glfw3.h>
#endif

#include "config.hpp"
#include "vkg/base/window/input.h"
#include <string>
#include <chrono>
#include <functional>

namespace vkg {
class Window {
public:
  static auto requiredExtensions() -> std::vector<const char *>;

public:
  explicit Window(const WindowConfig &windowConfig);
  auto createSurface(vk::Instance instance) -> void;

  auto windowShouldClose() -> bool;
  auto pollEvents() -> void;
  auto terminate() -> void;

  auto init() -> void;

  auto input() -> Input &;
  auto onMouseMove(int32_t x, int32_t y) -> void;
  auto onMouseScroll(double xoffset, double yoffset) -> void;
  auto onMouseButton(MouseButton mouseButton, bool press) -> void;
  auto onKey(Key key, bool pressed) -> void;
  auto onWindowResize(int32_t width, int32_t height) -> void;
  auto resizeWanted() const -> bool;
  auto setResizeWanted(bool resizeWanted) -> void;

  auto vkSurface() -> vk::SurfaceKHR;

  auto width() const -> uint32_t;
  auto height() const -> uint32_t;
  auto windowTitle() -> std::string;
  auto setWindowTitle(const std::string &title) -> void;

private:
  WindowConfig windowConfig;

  std::string title_;

  vk::UniqueSurfaceKHR surface;

  bool resizeWanted_{false};
  uint32_t width_, height_;

  Input input_{};

  uint32_t frames{0};
  std::chrono::time_point<std::chrono::high_resolution_clock> lastTime;

#if defined(USE_GLFW_WINDOW)
  GLFWwindow *window{nullptr};
#endif
};
}
