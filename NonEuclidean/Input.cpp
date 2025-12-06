#include "Input.h"
#include "GameHeader.h"
#include <cstring>

Input::Input() { memset(this, 0, sizeof(Input)); }

void Input::EndFrame() {
  memset(key_press, 0, sizeof(key_press));
  memset(mouse_button_press, 0, sizeof(mouse_button_press));
  mouse_dx = mouse_dx * GH_MOUSE_SMOOTH + mouse_ddx * (1.0f - GH_MOUSE_SMOOTH);
  mouse_dy = mouse_dy * GH_MOUSE_SMOOTH + mouse_ddy * (1.0f - GH_MOUSE_SMOOTH);
  mouse_ddx = 0.0f;
  mouse_ddy = 0.0f;
}
