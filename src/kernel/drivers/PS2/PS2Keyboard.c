#include "PS2Keyboard.h"

#include "arch/i686/io.h"
#include "arch/i686/irq.h"
#include "arch/i686/i8259.h"
#include "malloc.h"
void ps2_wait_input_clear() {
    /*
    while (i686_inb(PS2_STATUS) & 0x02); // Wait until input buffer clear
    */
}

void ps2_wait_output_fill() {
    // while (!(i686_inb(PS2_STATUS) & 0x01)); // Wait until output buffer full
}

void ps2_write(uint8_t port, uint8_t data) {
    /*
    ps2_wait_input_clear();
    i686_outb(port, data);
    */
}
uint8_t ps2_read(uint8_t port) {
    /*
    ps2_wait_output_fill();
    return i686_inb(port);
    */
    return 0;
}
uint8_t ps2_read_data() {
    /*
    ps2_wait_output_fill();
    return i686_inb(PS2_DATA);
    */
    return 0;
}

void ps2_init_keyboard() 
{
    /*
    ps2_wait_input_clear();
    ps2_write(PS2_COMMAND, 0x20); // Read command byte
    ps2_wait_output_fill();
    uint8_t config = ps2_read(PS2_DATA);
    config |= 0x01; // Set bit 0 to enable interrupts
    config &= ~0x10; // Clear bit 4 to disable translation
    ps2_wait_input_clear();
    ps2_write(PS2_COMMAND, 0x60); // Write command byte
    ps2_wait_input_clear();
    ps2_write(PS2_DATA, config); // Write new config
    
    ps2_write(PS2_COMMAND, 0xAE); // Enable keyboard
    ps2_wait_input_clear();
    */
}
