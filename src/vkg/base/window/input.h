#ifndef VKG_INPUT_H
#define VKG_INPUT_H
#include <cstdint>
#ifdef __cplusplus
extern "C" {
#endif

enum MouseButton {
  MouseButtonLeft,
  MouseButtonRight,
  MouseButtonMiddle,
  MouseButtonLast = MouseButtonMiddle
};

enum Key {
  KeySPACE,
  KeyAPOSTROPHE, /* ' */
  KeyCOMMA,      /* , */
  KeyMINUS,      /* - */
  KeyPERIOD,     /* . */
  KeySLASH,      /* / */
  Key0,
  Key1,
  Key2,
  Key3,
  Key4,
  Key5,
  Key6,
  Key7,
  Key8,
  Key9,
  KeySEMICOLON, /* ; */
  KeyEQUAL,     /* = */
  KeyA,
  KeyB,
  KeyC,
  KeyD,
  KeyE,
  KeyF,
  KeyG,
  KeyH,
  KeyI,
  KeyJ,
  KeyK,
  KeyL,
  KeyM,
  KeyN,
  KeyO,
  KeyP,
  KeyQ,
  KeyR,
  KeyS,
  KeyT,
  KeyU,
  KeyV,
  KeyW,
  KeyX,
  KeyY,
  KeyZ,
  KeyLEFT_BRACKET,  /* [ */
  KeyBACKSLASH,     /* \ */
  KeyRIGHT_BRACKET, /* ] */
  KeyGRAVE_ACCENT,  /* ` */
  KeyWORLD_1,       /* non-US #1 */
  KeyWORLD_2,       /* non-US #2 */

  /* FunctionKeys */
  KeyESCAPE,
  KeyENTER,
  KeyTAB,
  KeyBACKSPACE,
  KeyINSERT,
  KeyDELETE,
  KeyRIGHT,
  KeyLEFT,
  KeyDOWN,
  KeyUP,
  KeyPAGE_UP,
  KeyPAGE_DOWN,
  KeyHOME,
  KeyEND,
  KeyCAPS_LOCK,
  KeySCROLL_LOCK,
  KeyNUM_LOCK,
  KeyPRINT_SCREEN,
  KeyPAUSE,
  KeyF1,
  KeyF2,
  KeyF3,
  KeyF4,
  KeyF5,
  KeyF6,
  KeyF7,
  KeyF8,
  KeyF9,
  KeyF10,
  KeyF11,
  KeyF12,
  KeyF13,
  KeyF14,
  KeyF15,
  KeyF16,
  KeyF17,
  KeyF18,
  KeyF19,
  KeyF20,
  KeyF21,
  KeyF22,
  KeyF23,
  KeyF24,
  KeyF25,
  KeyKP_0,
  KeyKP_1,
  KeyKP_2,
  KeyKP_3,
  KeyKP_4,
  KeyKP_5,
  KeyKP_6,
  KeyKP_7,
  KeyKP_8,
  KeyKP_9,
  KeyKP_DECIMAL,
  KeyKP_DIVIDE,
  KeyKP_MULTIPLY,
  KeyKP_SUBTRACT,
  KeyKP_ADD,
  KeyKP_ENTER,
  KeyKP_EQUAL,
  KeyLEFT_SHIFT,
  KeyLEFT_CONTROL,
  KeyLEFT_ALT,
  KeyLEFT_SUPER,
  KeyRIGHT_SHIFT,
  KeyRIGHT_CONTROL,
  KeyRIGHT_ALT,
  KeyRIGHT_SUPER,
  KeyMENU,
  KeyLast = KeyMENU
};

typedef struct {
  int32_t mousePosX, mousePosY;
  double scrollXOffset;
  double scrollYOffset;
  bool mouseButtonPressed[MouseButtonLast + 1];
  bool keyPressed[KeyLast + 1];
} Input;

#ifdef __cplusplus
}
#endif
#endif //VKG_INPUT_H