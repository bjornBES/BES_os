#include "PS2_keyboard.h"

#include "drivers/Keyboard/keyboard.h"
#include "drivers/PS2/8042_controller.h"

#include "memory.h"
#include "debug.h"

#define MODULE "PS2_keyboard"

keyboardLEDs keyboard_leds;
keyboardKeys keyboard_keys;

void ps2_keyboard_process_key_value(uint32_t key_value)
{
    if (key_value > 0xFFFF)
    {
        return;
    }

    // key was pressed
    if ((key_value & 0xFF) < 0x80)
    {
        // check if this is not control key
        if (key_value == KEY_LEFT_SHIFT || key_value == KEY_RIGHT_SHIFT)
        {
            keyboard_keys.shift = true;
            keyboard_update_keys_state();
            return;
        }
        else if (key_value == KEY_LEFT_CTRL || key_value == KEY_RIGHT_CTRL)
        {
            keyboard_keys.ctrl = true;
            keyboard_update_keys_state();
            return;
        }
        else if (key_value == KEY_LEFT_ALT || key_value == KEY_RIGHT_ALT)
        {
            keyboard_keys.alt = true;
            keyboard_update_keys_state();
            return;
        }
        else if (key_value == KEY_CAPSLOCK)
        {
            keyboard_leds_state.capslock = ~keyboard_leds_state.capslock; // reverse state
            return;
        }
        else if (key_value == KEY_NUMBERLOCK)
        {
            keyboard_leds_state.numlock = ~keyboard_leds_state.numlock; // reverse state
            return;
        }
        else if (key_value == KEY_SCROLLLOCK)
        {
            keyboard_leds_state.scrollock = ~keyboard_leds_state.scrollock; // reverse state
            return;
        }

        // check if key is not already pressed
        for (uint32_t i = 0; i < 6; i++)
        {
            if (keyboard_keys.pressed_keys[i] == key_value)
            {
                return;
            }
        }
        // add key to list
        for (uint32_t i = 0; i < 5; i++)
        {
            keyboard_keys.pressed_keys[i + 1] = keyboard_keys.pressed_keys[i];
        }
        keyboard_keys.pressed_keys[0] = key_value;
    }
    else
    {
        // recalculate key value to pressed key value
        key_value = (key_value - 0x80);

        // check if this is not control key
        if (key_value == KEY_LEFT_SHIFT || key_value == KEY_RIGHT_SHIFT)
        {
            keyboard_keys.shift = false;
            return;
        }
        else if (key_value == KEY_LEFT_CTRL || key_value == KEY_RIGHT_CTRL)
        {
            keyboard_keys.ctrl = false;
            return;
        }
        else if (key_value == KEY_LEFT_ALT || key_value == KEY_RIGHT_ALT)
        {
            keyboard_keys.alt = false;
            return;
        }

        // check where do we have this key in list and remove it
        for (uint32_t i = 0; i < 6; i++)
        {
            if (keyboard_keys.pressed_keys[i] == key_value)
            {
                keyboard_keys.pressed_keys[i] = 0;
                return;
            }
        }
    }
}
void ps2_keyboard_set_leds()
{
    // create data that will be sended to keyboard
    uint8_t data = 0;
    if (keyboard_leds_state.scrollock == true)
    {
        data |= (1 << 0);
    }
    if (keyboard_leds_state.numlock == true)
    {
        data |= (1 << 1);
    }
    if (keyboard_leds_state.capslock == true)
    {
        data |= (1 << 2);
    }

    if (ps2_first_channel_device == PS2_CHANNEL_KEYBOARD_INITALIZED)
    {
        ps2_first_channel_device = PS2_CHANNEL_KEYBOARD_CONNECTED; // ACK will not be recognized as pressed key

        ps2_write_first_channel(0xED);
        if (ps2_first_channel_wait_for_ack() == true)
        {
            ps2_write_first_channel(data);
            if (ps2_first_channel_wait_for_ack() == false)
            {
                log_crit(MODULE, "PS/2: LED change data not ACKed");
            }
        }
        else
        {
            log_crit(MODULE, "PS/2: 0xED not ACKed");
        }

        ps2_first_channel_buffer_pointer = 0;
        ps2_first_channel_device = PS2_CHANNEL_KEYBOARD_INITALIZED; // keyboard can function normally
    }

    // update variables
    keyboard_leds.capslock = keyboard_leds_state.capslock;
    keyboard_leds.numlock = keyboard_leds_state.numlock;
    keyboard_leds.scrollock = keyboard_leds_state.scrollock;
}

void ps2_keyboard_cheak_leds()
{
    if (keyboard_leds.capslock != keyboard_leds_state.capslock ||
        keyboard_leds.numlock != keyboard_leds_state.numlock ||
        keyboard_leds.scrollock != keyboard_leds_state.scrollock)
    {
        ps2_keyboard_set_leds();
    }
}

void initalize_ps2_keyboard()
{
    memset(&keyboard_keys, 0, sizeof(keyboardKeys));
    memset(&keyboard_leds, 0, sizeof(keyboardLEDs));
    keyboard_leds.capslock = 0;
	keyboard_leds.numlock = 0;
	keyboard_leds.scrollock = 0;
	keyboard_keys.shift = 0;
	keyboard_keys.ctrl = 0;
	keyboard_keys.alt = 0;

    if (ps2_first_channel_device == PS2_CHANNEL_KEYBOARD_CONNECTED)
    {
        fprintf(VFS_FD_DEBUG, "PS/2: First channel keyboard connected\n");
        fprintf(VFS_FD_DEBUG, "PS/2: Sending 0xF4 to keyboard\n");
        ps2_write_first_channel(0xF4);
        if (ps2_first_channel_wait_for_ack() == true)
        {
            fprintf(VFS_FD_DEBUG, "PS/2: After waiting for ack\n");
            ps2_first_channel_buffer_pointer = 0;
            ps2_first_channel_device = PS2_CHANNEL_KEYBOARD_INITALIZED;
        }
        else
        {
            log_err(MODULE, "ERROR: PS/2: First channel keyboard not ACKed");
            printf("ERROR: PS/2: First channel keyboard not ACKed\n");
        }
    }

    if (ps2_first_channel_device == PS2_CHANNEL_KEYBOARD_INITALIZED || ps2_second_channel_device == PS2_CHANNEL_KEYBOARD_INITALIZED)
    {
        ps2_keyboard_cheak_leds();
    }

    fprintf(VFS_FD_DEBUG, "PS/2: Keyboard initialized\n");
}