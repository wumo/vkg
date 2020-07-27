#include "window.hpp"

namespace vkg {
Window::Window(const WindowConfig &windowConfig)
  : windowConfig{windowConfig},
    width_{windowConfig.width},
    height_{windowConfig.height},
    title_{windowConfig.title} {
  init();
}

auto Window::vkSurface() -> vk::SurfaceKHR { return *surface; }

auto Window::width() const -> uint32_t { return width_; }
auto Window::height() const -> uint32_t { return height_; }
auto Window::windowTitle() -> std::string { return title_; }
auto Window::isVsync() const -> bool { return windowConfig.vsync; }

auto Window::input() -> Input & { return input_; }
auto Window::onMouseMove(int32_t x, int32_t y) -> void {
  input_.mousePosX = x;
  input_.mousePosY = y;
}
auto Window::onMouseScroll(double xoffset, double yoffset) -> void {
  input_.scrollXOffset = xoffset;
  input_.scrollYOffset = yoffset;
}
auto Window::onMouseButton(MouseButton mouseButton, bool press) -> void {
  input_.mouseButtonPressed[mouseButton] = press;
}
auto Window::onKey(Key key, bool pressed) -> void { input_.keyPressed[key] = pressed; }
auto Window::onWindowResize(int32_t width, int32_t height) -> void {
  resizeWanted_ = true;
  width_ = width;
  height_ = height;
}
auto Window::setResizeWanted(bool resizeWanted) -> void { resizeWanted_ = resizeWanted; }
auto Window::resizeWanted() const -> bool { return resizeWanted_; }
}