#include "vkg/base/window.hpp"
#include <unordered_map>
#include "vkg/util/syntactic_sugar.hpp"

namespace vkg {
static const std::unordered_map<int, Key> GLFW_Keys = {
  {GLFW_KEY_SPACE, KeySPACE},
  {GLFW_KEY_APOSTROPHE, KeyAPOSTROPHE}, /* ' */
  {GLFW_KEY_COMMA, KeyCOMMA},           /* , */
  {GLFW_KEY_MINUS, KeyMINUS},           /* - */
  {GLFW_KEY_PERIOD, KeyPERIOD},         /* . */
  {GLFW_KEY_SLASH, KeySLASH},           /* / */
  {GLFW_KEY_0, Key0},
  {GLFW_KEY_1, Key1},
  {GLFW_KEY_2, Key2},
  {GLFW_KEY_3, Key3},
  {GLFW_KEY_4, Key4},
  {GLFW_KEY_5, Key5},
  {GLFW_KEY_6, Key6},
  {GLFW_KEY_7, Key7},
  {GLFW_KEY_8, Key8},
  {GLFW_KEY_9, Key9},
  {GLFW_KEY_SEMICOLON, KeySEMICOLON}, /* ; */
  {GLFW_KEY_EQUAL, KeyEQUAL},         /* = */
  {GLFW_KEY_A, KeyA},
  {GLFW_KEY_B, KeyB},
  {GLFW_KEY_C, KeyC},
  {GLFW_KEY_D, KeyD},
  {GLFW_KEY_E, KeyE},
  {GLFW_KEY_F, KeyF},
  {GLFW_KEY_G, KeyG},
  {GLFW_KEY_H, KeyH},
  {GLFW_KEY_I, KeyI},
  {GLFW_KEY_J, KeyJ},
  {GLFW_KEY_K, KeyK},
  {GLFW_KEY_L, KeyL},
  {GLFW_KEY_M, KeyM},
  {GLFW_KEY_N, KeyN},
  {GLFW_KEY_O, KeyO},
  {GLFW_KEY_P, KeyP},
  {GLFW_KEY_Q, KeyQ},
  {GLFW_KEY_R, KeyR},
  {GLFW_KEY_S, KeyS},
  {GLFW_KEY_T, KeyT},
  {GLFW_KEY_U, KeyU},
  {GLFW_KEY_V, KeyV},
  {GLFW_KEY_W, KeyW},
  {GLFW_KEY_X, KeyX},
  {GLFW_KEY_Y, KeyY},
  {GLFW_KEY_Z, KeyZ},
  {GLFW_KEY_LEFT_BRACKET, KeyLEFT_BRACKET},   /* [ */
  {GLFW_KEY_BACKSLASH, KeyBACKSLASH},         /* \ */
  {GLFW_KEY_RIGHT_BRACKET, KeyRIGHT_BRACKET}, /* ] */
  {GLFW_KEY_GRAVE_ACCENT, KeyGRAVE_ACCENT},   /* ` */
  {GLFW_KEY_WORLD_1, KeyWORLD_1},             /* non-US #1 */
  {GLFW_KEY_WORLD_2, KeyWORLD_2},             /* non-US #2 */

  /* FunctionKeys */
  {GLFW_KEY_ESCAPE, KeyESCAPE},
  {GLFW_KEY_ENTER, KeyENTER},
  {GLFW_KEY_TAB, KeyTAB},
  {GLFW_KEY_BACKSPACE, KeyBACKSPACE},
  {GLFW_KEY_INSERT, KeyINSERT},
  {GLFW_KEY_DELETE, KeyDELETE},
  {GLFW_KEY_RIGHT, KeyRIGHT},
  {GLFW_KEY_LEFT, KeyLEFT},
  {GLFW_KEY_DOWN, KeyDOWN},
  {GLFW_KEY_UP, KeyUP},
  {GLFW_KEY_PAGE_UP, KeyPAGE_UP},
  {GLFW_KEY_PAGE_DOWN, KeyPAGE_DOWN},
  {GLFW_KEY_HOME, KeyHOME},
  {GLFW_KEY_END, KeyEND},
  {GLFW_KEY_CAPS_LOCK, KeyCAPS_LOCK},
  {GLFW_KEY_SCROLL_LOCK, KeySCROLL_LOCK},
  {GLFW_KEY_NUM_LOCK, KeyNUM_LOCK},
  {GLFW_KEY_PRINT_SCREEN, KeyPRINT_SCREEN},
  {GLFW_KEY_PAUSE, KeyPAUSE},
  {GLFW_KEY_F1, KeyF1},
  {GLFW_KEY_F2, KeyF2},
  {GLFW_KEY_F3, KeyF3},
  {GLFW_KEY_F4, KeyF4},
  {GLFW_KEY_F5, KeyF5},
  {GLFW_KEY_F6, KeyF6},
  {GLFW_KEY_F7, KeyF7},
  {GLFW_KEY_F8, KeyF8},
  {GLFW_KEY_F9, KeyF9},
  {GLFW_KEY_F10, KeyF10},
  {GLFW_KEY_F11, KeyF11},
  {GLFW_KEY_F12, KeyF12},
  {GLFW_KEY_F13, KeyF13},
  {GLFW_KEY_F14, KeyF14},
  {GLFW_KEY_F15, KeyF15},
  {GLFW_KEY_F16, KeyF16},
  {GLFW_KEY_F17, KeyF17},
  {GLFW_KEY_F18, KeyF18},
  {GLFW_KEY_F19, KeyF19},
  {GLFW_KEY_F20, KeyF20},
  {GLFW_KEY_F21, KeyF21},
  {GLFW_KEY_F22, KeyF22},
  {GLFW_KEY_F23, KeyF23},
  {GLFW_KEY_F24, KeyF24},
  {GLFW_KEY_F25, KeyF25},
  {GLFW_KEY_KP_0, KeyKP_0},
  {GLFW_KEY_KP_1, KeyKP_1},
  {GLFW_KEY_KP_2, KeyKP_2},
  {GLFW_KEY_KP_3, KeyKP_3},
  {GLFW_KEY_KP_4, KeyKP_4},
  {GLFW_KEY_KP_5, KeyKP_5},
  {GLFW_KEY_KP_6, KeyKP_6},
  {GLFW_KEY_KP_7, KeyKP_7},
  {GLFW_KEY_KP_8, KeyKP_8},
  {GLFW_KEY_KP_9, KeyKP_9},
  {GLFW_KEY_KP_DECIMAL, KeyKP_DECIMAL},
  {GLFW_KEY_KP_DIVIDE, KeyKP_DIVIDE},
  {GLFW_KEY_KP_MULTIPLY, KeyKP_MULTIPLY},
  {GLFW_KEY_KP_SUBTRACT, KeyKP_SUBTRACT},
  {GLFW_KEY_KP_ADD, KeyKP_ADD},
  {GLFW_KEY_KP_ENTER, KeyKP_ENTER},
  {GLFW_KEY_KP_EQUAL, KeyKP_EQUAL},
  {GLFW_KEY_LEFT_SHIFT, KeyLEFT_SHIFT},
  {GLFW_KEY_LEFT_CONTROL, KeyLEFT_CONTROL},
  {GLFW_KEY_LEFT_ALT, KeyLEFT_ALT},
  {GLFW_KEY_LEFT_SUPER, KeyLEFT_SUPER},
  {GLFW_KEY_RIGHT_SHIFT, KeyRIGHT_SHIFT},
  {GLFW_KEY_RIGHT_CONTROL, KeyRIGHT_CONTROL},
  {GLFW_KEY_RIGHT_ALT, KeyRIGHT_ALT},
  {GLFW_KEY_RIGHT_SUPER, KeyRIGHT_SUPER},
  {GLFW_KEY_MENU, KeyMENU},
};

static void errorFunc(int error, const char *msg) {
  println("glfw error: [", error, "] msg: ", msg);
}

auto Window::requiredExtensions() -> std::vector<const char *> {
  errorIf(!glfwInit(), "fail to init glfw");
  errorIf(!glfwVulkanSupported(), "glfw glfw doesn't support vulkan");

  glfwSetErrorCallback(errorFunc);

  uint32_t glfwExtensionCount = 0;
  auto *glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

  return std::vector<const char *>(glfwExtensions, glfwExtensions + glfwExtensionCount);
}

static void mouse_move_callback(GLFWwindow *window, double xpos, double ypos) {
  auto *window_ = static_cast<Window *>(glfwGetWindowUserPointer(window));
  window_->onMouseMove(int32_t(xpos), int32_t(ypos));
}

static auto scroll_callback(GLFWwindow *window, double xoffset, double yoffset) -> void {
  auto *window_ = static_cast<Window *>(glfwGetWindowUserPointer(window));
  window_->onMouseScroll(xoffset, yoffset);
}

static void mouse_button_callback(GLFWwindow *window, int button, int action, int mods) {
  auto *window_ = static_cast<Window *>(glfwGetWindowUserPointer(window));
  MouseButton btn;
  switch(button) {
    case GLFW_MOUSE_BUTTON_LEFT: btn = MouseButtonLeft; break;
    case GLFW_MOUSE_BUTTON_MIDDLE: btn = MouseButtonMiddle; break;
    case GLFW_MOUSE_BUTTON_RIGHT: btn = MouseButtonRight; break;
    default: return;
  }
  window_->onMouseButton(btn, action == GLFW_PRESS);
}

static auto key_callback(GLFWwindow *window, int key, int, int action, int) -> void {
  auto *window_ = static_cast<Window *>(glfwGetWindowUserPointer(window));
  if(key > GLFW_KEY_LAST || key < 0 || !GLFW_Keys.contains(key)) return;

  if(action == GLFW_PRESS) window_->onKey(GLFW_Keys.at(key), true);
  else if(action == GLFW_RELEASE)
    window_->onKey(GLFW_Keys.at(key), false);
}

static void resize_callback(GLFWwindow *window, int w, int h) {
  auto *window_ = static_cast<Window *>(glfwGetWindowUserPointer(window));
  window_->onWindowResize(w, h);
}

auto Window::init() -> void {
  errorIf(!glfwInit(), "fail to init glfw");
  errorIf(!glfwVulkanSupported(), "glfw glfw doesn't support vulkan");

  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  window = glfwCreateWindow(
    windowConfig.width, windowConfig.height, windowConfig.title.c_str(), nullptr,
    nullptr);
  glfwSetWindowUserPointer(window, this);

  glfwSetInputMode(window, GLFW_STICKY_KEYS, GLFW_TRUE);
  glfwSetCursorPosCallback(window, mouse_move_callback);
  glfwSetMouseButtonCallback(window, mouse_button_callback);
  glfwSetScrollCallback(window, scroll_callback);
  glfwSetKeyCallback(window, key_callback);
  glfwSetFramebufferSizeCallback(window, resize_callback);
}

auto Window::setWindowTitle(const std::string &title) -> void {
  title_ = title;
  glfwSetWindowTitle(window, title_.c_str());
}

auto Window::createSurface(vk::Instance instance) -> void {
  VkSurfaceKHR _surface;
  auto result = glfwCreateWindowSurface(instance, window, nullptr, &_surface);
  errorIf(result != VK_SUCCESS, "error create surface");
  surface = vk::UniqueSurfaceKHR{
    _surface,
    vk::ObjectDestroy<vk::Instance, VULKAN_HPP_DEFAULT_DISPATCHER_TYPE>{instance}};
}

auto Window::windowShouldClose() -> bool { return glfwWindowShouldClose(window); }
auto Window::pollEvents() -> void { glfwPollEvents(); }

auto Window::terminate() -> void {
  glfwDestroyWindow(window);
  glfwTerminate();
}
}