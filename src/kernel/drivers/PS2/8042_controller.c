#include "8042_controller.h"

#include "arch/i686/io.h"
#include "arch/i686/irq.h"
#include "arch/i686/i8259.h"
#include "arch/i686/pit.h"

#include "malloc.h"
#include "stdio.h"
#include "debug.h"
#include "memory.h"

#include "PS2_mouse.h"
#include "PS2_keyboard.h"

#include "drivers/mouse/mouse.h"

#include <printfDriver/printf.h>

#define MODULE "8042 Driver"

#define DEVICE_PRESENCE_IS_NOT_KNOWN 0xFF
#define DEVICE_NOT_PRESENT 0
#define DEVICE_PRESENT 1
#define DEVICE_PRESENT_BUT_ERROR_STATE 2

uint8_t ps2_first_channel_buffer[10];
uint8_t ps2_second_channel_buffer[10];

uint8_t ps2_controller_present;
uint8_t ps2_first_channel_present;
uint8_t ps2_second_channel_present;
uint8_t ps2_first_channel_device;
uint8_t ps2_second_channel_device;
uint8_t ps2_first_channel_buffer_pointer;
uint8_t ps2_second_channel_buffer_pointer;

bool fullyEnabled = false;

uint32_t ticks;

void ps2_wait_input_clear()
{
    ticks = timer_ticks;
    while (timer_ticks < 250 + ticks && i686_inb(PS2_STATUS) & 0x02)
        ; // Wait until input buffer clear
}
void ps2_wait_output_fill()
{
    while (!(i686_inb(PS2_STATUS) & 0x01))
        ; // Wait until output buffer full
}
void ps2_write(uint8_t port, uint8_t data)
{
    ps2_wait_input_clear();
    i686_outb(port, data);
}
uint8_t ps2_read(uint8_t port)
{
    ps2_wait_output_fill();
    return i686_inb(port);
}

void ps2_write_data(uint8_t data)
{
    ps2_write(PS2_DATA, data);
}
void ps2_write_cmd(uint8_t cmd)
{
    ps2_write(PS2_COMMAND, cmd);
}
uint8_t ps2_read_data()
{
    return ps2_read(PS2_DATA);
}

bool poll_timer()
{
    for (uint8_t i = 0; i < 255; i++)
    {
        uint8_t status = ps2_read(PS2_STATUS);
        if ((status & 0x02) == 0)
        {
            return true;
        }
    }

    // error
    printf("ERROR: Timer ran out in polling\n");
    return false;
}

void ps2_write_first_channel(uint8_t data)
{
    ps2_first_channel_buffer_pointer = 0;
    ps2_first_channel_buffer[0] = 0;
    ps2_write_data(data);
}

uint8_t ps2_first_channel_wait_for_ack(void)
{
    ticks = timer_ticks;
    uint32_t COUNT_TIMER = 0;
    while (timer_ticks < 250 + ticks)
    {
        uint8_t status = ps2_read(PS2_STATUS);
        // fprintf(VFS_FD_DEBUG, "status = 0x%X\n", status);
        if ((status & 0x01))
        {
            uint8_t data = ps2_read_data();
            if (data == 0xFA)
            {
                return true; // acknowledge sended
            }
            else
            {
                return false; // acknowledge was not sended
            }
        }
        COUNT_TIMER++;
        if (COUNT_TIMER > 1000)
        {
            printf("ERROR: Timer ran out in polling\n");
            return false;
        }
    }
    return false;
}

uint8_t ps2_first_channel_wait_for_response(void)
{
    ticks = timer_ticks;
    while (timer_ticks < 250 + ticks)
    {
        uint8_t status = ps2_read(PS2_STATUS);
        // fprintf(VFS_FD_DEBUG, "status = 0x%X\n", status);
        if ((status & 0x01))
        {
            return true;
        }
    }

    return false;
}

void ps2_write_second_channel(uint8_t data)
{
    ps2_second_channel_buffer_pointer = 0;
    ps2_second_channel_buffer[0] = 0;
    ps2_write_cmd(0xD4);
    ps2_write_data(data);
}

uint8_t ps2_second_channel_wait_for_ack(void)
{
    ticks = timer_ticks;
    while (timer_ticks < 250 + ticks)
    {
        uint8_t status = ps2_read(PS2_STATUS);
        // fprintf(VFS_FD_DEBUG, "status = 0x%X\n", status);
        if ((status & 0x10) || ps2_second_channel_buffer_pointer > 0)
        {
            uint8_t data = ps2_read_data();
            if (data == 0xFA || ps2_second_channel_buffer[0] == 0xFA)
            {
                return true; // acknowledge sended
            }
            else
            {
                return false; // acknowledge was not sended
            }
        }
    }
    return false;
}

uint8_t ps2_second_channel_wait_for_response(void)
{
    ticks = timer_ticks;
    while (timer_ticks < 250 + ticks)
    {
        uint8_t status = ps2_read(PS2_STATUS);
        // fprintf(VFS_FD_DEBUG, "status = 0x%X\n", status);
        if ((status & 0x10))
        {
            return true;
        }
        if (ps2_second_channel_buffer_pointer > 1)
        {
            return true;
        }
    }

    return false;
}

void ps2_first_channel_handler(Registers *regs)
{
    uint8_t status = i686_inb(PS2_STATUS);
    // check if there are data
    if ((status & 0x1) != 0x1)
    {
        return;
    }

    // read data
    ps2_first_channel_buffer[ps2_first_channel_buffer_pointer] = i686_inb(PS2_DATA);

    // move pointer
    ps2_first_channel_buffer_pointer++;
    if (ps2_first_channel_buffer_pointer >= 10)
    {
        printf("ERROR: PS/2 first channel buffer is full\n");
        log_err(MODULE, "ERROR: PS/2 first channel buffer is full");
        ps2_first_channel_buffer_pointer = 0;
    }

    // process data
    if (ps2_first_channel_device == PS2_CHANNEL_KEYBOARD_INITALIZED)
    {
        uint32_t key_value = 0;

        // if data starts with 0xE1, it means that keyboard will send two more bytes
        if (ps2_first_channel_buffer[0] == 0xE1)
        {
            if (ps2_first_channel_buffer_pointer >= 3)
            {
                key_value = (ps2_first_channel_buffer[0] | ps2_first_channel_buffer[1] << 8 | 0xE1 << 16);
            }
            else
            {
                return; // wait for rest of data
            }
        }
        // if data starts with 0xE0, it means that keyboard will send one more byte
        else if (ps2_first_channel_buffer[0] == 0xE0)
        {
            if (ps2_first_channel_buffer_pointer >= 2)
            {
                key_value = (ps2_first_channel_buffer[1] | 0xE0 << 8);
            }
            else
            {
                return; // wait for rest of data
            }
        }
        // normal data in one byte
        else
        {
            key_value = ps2_first_channel_buffer[0];
        }

        // process pressed key
        keyboard_process_code(key_value);

        // save key to list
        ps2_keyboard_process_key_value(key_value);

        // clear variables
        ps2_first_channel_buffer_pointer = 0;
    }
    i8259_SendEndOfInterrupt(1);
}
void ps2_second_channel_handler(Registers *regs)
{
    /*
    // check if there are data
    if ((i686_inb(PS2_STATUS) & 0x1) != 0x1)
    {
        return;
    }

    // read data
    ps2_second_channel_buffer[ps2_second_channel_buffer_pointer] = i686_inb(PS2_DATA);

    // move pointer
    ps2_second_channel_buffer_pointer++;
    if (ps2_second_channel_buffer_pointer >= 10)
    {
        printf("ERROR: PS/2 second channel buffer is full\n");
        log_err(MODULE, "ERROR: PS/2 second channel buffer is full");
        ps2_second_channel_buffer_pointer = 0;
    }

    // process data
    if (ps2_second_channel_device == PS2_CHANNEL_MOUSE_INITALIZED && ps2_second_channel_buffer_pointer >= ps2_second_channel_mouse_data_bytes)
    {
        // if program waits for data from mouse, process it
        if (ps2_mouse_enable == true)
        {
            // buttons
            mouse_buttons = ps2_second_channel_buffer[0];

            // X movement
            if (ps2_second_channel_buffer[1] < 0x80)
            {
                mouse_movement_x = ps2_second_channel_buffer[1];
            }
            else
            {
                mouse_movement_x = (0xFFFFFFFF - (0xFF - ps2_second_channel_buffer[1]));
            }

            // Y movement
            if (ps2_second_channel_buffer[2] < 0x80)
            {
                mouse_movement_y = (0xFFFFFFFF - ps2_second_channel_buffer[2] + 1);
            }
            else
            {
                mouse_movement_y = (0x100 - ps2_second_channel_buffer[2]);
            }

            // wheel
            if (ps2_second_channel_mouse_data_bytes == 4 && ps2_second_channel_buffer[3] != 0)
            {
                if (ps2_second_channel_buffer[3] < 0x80)
                {
                    mouse_wheel_movement = (0xFFFFFFFF - ps2_second_channel_buffer[3]);
                }
                else
                {
                    mouse_wheel_movement = (0x100 - ps2_second_channel_buffer[3]);
                }
            }

            // click button state
            mouse_update_click_button_state();

            // inform method wait_for_user_input() from source/drivers/system/user_wait.c that there was received packet from mouse
            mouse_event = true;
        }

        // reset variable
        ps2_second_channel_buffer_pointer = 0;
    }
    */
    i8259_SendEndOfInterrupt(12);
}

void ps2_init_keyboard()
{
    fullyEnabled = false;
    // initalize variables
    ps2_first_channel_present = DEVICE_PRESENCE_IS_NOT_KNOWN;
    ps2_first_channel_device = PS2_CHANNEL_NO_DEVICE_CONNECTED;
    ps2_first_channel_buffer_pointer = 0;
    ps2_second_channel_present = DEVICE_PRESENCE_IS_NOT_KNOWN;
    ps2_second_channel_device = PS2_CHANNEL_NO_DEVICE_CONNECTED;
    ps2_second_channel_buffer_pointer = 0;

    // disable PS/2 channels
    ps2_write_cmd(0xAD);
    ps2_write_cmd(0xA7);

    for (uint16_t i = 0; i < 1000; i++)
    {
        if ((i686_inb(PS2_STATUS) & 0x01))
        {
            i686_inb(PS2_DATA);
        }
    }

    if ((i686_inb(PS2_STATUS) & 0x1))
    { // error: data port was not cleared
        printf("ERROR: locked data port\n");
        log_err(MODULE, "ERROR: locked data port");
        return;
    }

    // disable interrupts of both channels
    ps2_write_cmd(0x20);
    uint8_t controller_configuration_byte = ps2_read_data();
    ps2_write_cmd(0x60);
    ps2_write_data(controller_configuration_byte & ~((1 << 0) | (1 << 1)));

    // check presence of channels
    if ((controller_configuration_byte & (1 << 4)) == 0)
    { // first channel should be disabled, if it is not, it can not exist
        ps2_first_channel_present = DEVICE_NOT_PRESENT;
    }
    if ((controller_configuration_byte & (1 << 5)) == 0)
    { // second channel should be disabled, if it is not, it can not exist
        ps2_second_channel_present = DEVICE_NOT_PRESENT;
    }

    // perform controller self-test
    ps2_write_cmd(0xAA);
    if (ps2_read_data() == 0x55)
    {
        // self-test succesfull
        printf("Self-test succesfull\n");
    }
    else
    {
        // self-test failed
        printf("ERROR: Self-test failed\n");
        log_err(MODULE, "ERROR: Self-test failed");
        ps2_first_channel_present = DEVICE_NOT_PRESENT;
        ps2_second_channel_present = DEVICE_NOT_PRESENT;
        return;
    }

    // enable PS/2 channels
    ps2_write_cmd(0xAE);
    ps2_write_cmd(0xA8);

    // check presence of channels
    ps2_write_cmd(0x20);
    controller_configuration_byte = ps2_read_data();
    if (ps2_first_channel_present == DEVICE_PRESENCE_IS_NOT_KNOWN)
    { // we do not know first channel presence yet
        if ((controller_configuration_byte & (1 << 4)) == 0)
        {
            ps2_first_channel_present = DEVICE_PRESENT; // first channel is enabled, so it exist
        }
        else
        {
            ps2_first_channel_present = DEVICE_NOT_PRESENT; // first channel is disabled, so it do not exist
        }
    }
    if (ps2_second_channel_present == DEVICE_PRESENCE_IS_NOT_KNOWN)
    { // we do not know second channel presence yet
        if ((controller_configuration_byte & (1 << 5)) == 0)
        {
            ps2_second_channel_present = DEVICE_PRESENT; // second channel is enabled, so it exist
        }
        else
        {
            ps2_second_channel_present = DEVICE_NOT_PRESENT; // second channel is disabled, so it do not exist
        }
    }

    // disable PS/2 channels
    ps2_write_cmd(0xAD);
    ps2_write_cmd(0xA7);

    // test first channel
    if (ps2_first_channel_present == DEVICE_PRESENT)
    {
        ps2_write_cmd(0xAB);
        if (ps2_read_data() == 0x00)
        { // test successfull
            printf("First channel test successfull\n");
        }
        else
        { // test failed
            printf("ERROR: First channel test failed\n");
            log_err(MODULE, "ERROR: First channel test failed");
            ps2_first_channel_present = DEVICE_PRESENT_BUT_ERROR_STATE;
        }
    }

    // test second channel
    if (ps2_second_channel_present == DEVICE_PRESENT)
    {
        ps2_write_cmd(0xA9);
        if (ps2_read_data() == 0x00)
        { // test successfull
            printf("Second channel test successfull\n");
        }
        else
        { // test failed
            printf("ERROR: Second channel test failed\n");
            log_err(MODULE, "ERROR: Second channel test failed");
            ps2_second_channel_present = DEVICE_PRESENT_BUT_ERROR_STATE;
        }
    }

    // enable interrupts
    ps2_write_cmd(0x20);
    controller_configuration_byte = ps2_read_data();
    if (ps2_first_channel_present == DEVICE_PRESENT)
    {
        i686_IRQ_RegisterHandler(1, ps2_first_channel_handler);
        controller_configuration_byte |= (1 << 6);  // enable translation to scan code set 1
        controller_configuration_byte &= ~(1 << 0); // enable interrupts
    }
    if (ps2_second_channel_present == DEVICE_PRESENT)
    {
        i686_IRQ_RegisterHandler(12, ps2_second_channel_handler);
        controller_configuration_byte &= ~(1 << 1); // enable interrupt
    }
    ps2_write_cmd(0x60);
    ps2_write_data(controller_configuration_byte);

    // enable channels
    if (ps2_first_channel_present == DEVICE_PRESENT)
    {
        ps2_write_cmd(0xAE);
        ps2_write_first_channel(0xFF); // reset
        if (ps2_first_channel_wait_for_ack() == false)
        {
            printf("ERROR: First channel no ACK after reset\n");
            log_err(MODULE, "ERROR: First channel no ACK after reset");
        }
        if (ps2_first_channel_wait_for_response() == false)
        {
            printf("ERROR: First channel no response after reset\n");
            log_err(MODULE, "ERROR: First channel no response after reset");
        }
    }
    if (ps2_second_channel_present == DEVICE_PRESENT)
    {
        ps2_write_cmd(0xA8);
        ps2_write_second_channel(0xFF); // reset
        if (ps2_second_channel_wait_for_ack() == false)
        {
            printf("ERROR: Second channel no ACK after reset\n");
            log_err(MODULE, "ERROR: Second channel no ACK after reset");
        }
        if (ps2_second_channel_wait_for_response() == false)
        {
            printf("ERROR: Second channel no response after reset\n");
            log_err(MODULE, "ERROR: Second channel no response after reset");
        }
    }
    log_debug(MODULE, "enabled channels");

    // check what type of device is connected on first channel
    if (ps2_first_channel_present == DEVICE_PRESENT)
    {
        // after reset device should not be in streaming mode, but to be sure disable streaming
        ps2_write_first_channel(0xF5);
        if (ps2_first_channel_wait_for_ack() == false)
        {
            printf("ERROR: First channel no ACK after 0xF5\n");
            log_err(MODULE, "ERROR: First channel no ACK after 0xF5");
        }

        // read device ID
        ps2_write_first_channel(0xF2);
        if (ps2_first_channel_wait_for_ack() == true)
        {
            if (ps2_first_channel_wait_for_response() == true)
            {
                uint8_t data = ps2_read_data();

                // Some devices send a second ID byte, not always.
                if (ps2_first_channel_wait_for_response())
                {
                    ps2_read_data();
                }

                if (data == 0xAA || data == 0xAB || data == 0xAC)
                {
                    ps2_first_channel_device = PS2_CHANNEL_KEYBOARD_CONNECTED;
                }
                else
                {
                    ps2_first_channel_device = data; // unknown device
                }

                ps2_first_channel_buffer_pointer = 0;
                ps2_first_channel_buffer[0] = 0;
                ps2_write_first_channel(0xF4);
                if (ps2_first_channel_wait_for_ack() == true)
                {
                    printf("ERROR: channel 1 init\n");
                    ps2_first_channel_buffer_pointer = 0;
                    ps2_first_channel_device = PS2_CHANNEL_KEYBOARD_INITALIZED;
                }
                else
                {
                    log_err(MODULE, "ERROR: PS/2: First channel keyboard not ACKed");
                    printf("ERROR: PS/2: First channel keyboard not ACKed\n");
                }
                /*
                 */
                // enable interrupts
                if (ps2_first_channel_present == DEVICE_PRESENT)
                {
                    controller_configuration_byte |= (1 << 0); // enable interrupts
                }

                printf("First channel device ID: %x", data);
            }
            else
            {
                printf("ERROR: First channel device did not send ID data\n");
                log_err(MODULE, "ERROR: First channel device did not send ID data\n");
            }
        }
        else
        {
            printf("ERROR: First channel device did not send ID ack\n");
            log_err(MODULE, "ERROR: First channel device did not send ID ack\n");
        }
    }
    printf("checking second channel\n");

    // check what type of device is connected on second channel
    /*
    if (ps2_second_channel_present == DEVICE_PRESENT)
    {
        // after reset device should not be in streaming mode, but to be sure disable streaming
        ps2_write_second_channel(0xF5);
        if (ps2_second_channel_wait_for_ack() == false)
        {
            printf("ERROR: Second channel no ACK after 0xF5\n");
        }

        // read device ID
        ps2_second_channel_buffer[1] = 0xFF;
        ps2_write_second_channel(0xF2);
        if (ps2_second_channel_wait_for_ack() == true)
        {
            if (ps2_second_channel_wait_for_response() == true)
            {
                if (ps2_second_channel_buffer[1] == 0x00 || ps2_second_channel_buffer[1] == 0x03 || ps2_second_channel_buffer[1] == 0x04)
                {
                    ps2_second_channel_device = PS2_CHANNEL_MOUSE_CONNECTED;
                }
                else
                {
                    ps2_second_channel_device = ps2_second_channel_buffer[1]; // unknown device
                }

                ps2_write_second_channel(0xF4);
                ps2_second_channel_buffer[0] = 0;
                ps2_second_channel_buffer_pointer = 0;
                if (ps2_second_channel_wait_for_ack() == true)
                {
                    ps2_second_channel_buffer_pointer = 0;
                    ps2_second_channel_device = PS2_CHANNEL_KEYBOARD_INITALIZED;
                }

                // enable interrupts
                if (ps2_second_channel_present == DEVICE_PRESENT)
                {
                    controller_configuration_byte |= (1 << 1); // enable interrupt
                }
            }
            else
            {
                printf("ERROR: Second channel device did not send ID data\n");
            }
        }
        else
        {
            printf("ERROR: Second channel device did not send ID ack\n");
        }
    }
    */
    printf("sending 0x60 to CMD\n");
    ps2_write_cmd(0x60);
    printf("sending %u to DATA\n", controller_configuration_byte);
    ps2_write_data(controller_configuration_byte);

    printf("first channel present %u\n", ps2_first_channel_present);
    if (ps2_first_channel_present == DEVICE_PRESENT)
    {
        initalize_ps2_keyboard();
    }

    fullyEnabled = true;
    fprintf(VFS_FD_STDOUT, "controller %u channels %u %u", ps2_controller_present, ps2_first_channel_device, ps2_second_channel_device);
}
