#ifndef VKG_C_WINDOW_H
#define VKG_C_WINDOW_H

#include "vkg/base/window/input.h"

#include <cstdint>
#ifdef __cplusplus
extern "C" {
#endif

struct CWindow;
typedef struct CWindow CWindow;

uint32_t WindowGetWidth(CWindow *window);
uint32_t WindowGetHeight(CWindow *window);
void WindowSetTitle(CWindow *window, char *titleBuf, uint32_t size);
Input *WindowGetInput(CWindow *window);

int32_t InputMousePosX(Input *input);
int32_t InputMousePosY(Input *input);
double InputScrollOffsetX(Input *input);
double InputScrollOffsety(Input *input);
bool InputMouseButtonPressed(Input *input, MouseButton btn);
bool InputKeyPressed(Input *input, Key key);

#ifdef __cplusplus
}
#endif

#endif //VKG_C_WINDOW_H
