#pragma once

#include <stdint.h>

#define MOUSE_NO_DRAG 0
#define MOUSE_NO_EVENT 0
#define MOUSE_CLICK 1
#define MOUSE_DRAG 2

extern uint32_t mouse_event;
extern uint32_t mouse_click_button_state, mouse_buttons, mouse_movement_x, mouse_movement_y, mouse_wheel_movement;