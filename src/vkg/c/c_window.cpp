#include "c_window.h"
#include "vkg/base/window.hpp"

using namespace vkg;
uint32_t WindowGetWidth(CWindow *window) {
  auto *win = reinterpret_cast<Window *>(window);
  return win->width();
}
uint32_t WindowGetHeight(CWindow *window) {
  auto *win = reinterpret_cast<Window *>(window);
  return win->height();
}
uint32_t WindowGetTitleLength(CWindow *window) {
  auto *win = reinterpret_cast<Window *>(window);
  return uint32_t(win->windowTitle().size());
}
void WindowGetTitle(CWindow *window, char *titleBuf) {
  auto *win = reinterpret_cast<Window *>(window);
  auto title = win->windowTitle();
  memcpy(titleBuf, title.c_str(), title.size());
}
void WindowSetTitle(CWindow *window, char *titleBuf, uint32_t size) {
  auto *win = reinterpret_cast<Window *>(window);
  win->setWindowTitle(std::string{titleBuf, size});
}
Input *WindowGetInput(CWindow *window) {
  auto *win = reinterpret_cast<Window *>(window);
  return &win->input();
}
int32_t InputMousePosX(Input *input) { return input->mousePosX; }
int32_t InputMousePosY(Input *input) { return input->mousePosY; }
double InputScrollOffsetX(Input *input) { return input->scrollXOffset; }
double InputScrollOffsety(Input *input) { return input->scrollYOffset; }
bool InputMouseButtonPressed(Input *input, MouseButton btn) {
  return input->mouseButtonPressed[btn];
}
bool InputKeyPressed(Input *input, Key key) { return input->keyPressed[key]; }
