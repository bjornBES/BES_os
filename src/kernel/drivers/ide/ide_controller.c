
#include "ide_controller.h"

#include "arch/i686/io.h"
#include "arch/i686/i8259.h"
#include "arch/i686/pit.h"

#include "debug.h"
#include "memory.h"
#include "stdio.h"

#define MODULE "IDE"

#define CHS 0
#define LBA28 1
#define LBA48 2

#define NoDMA 0
#define HasDMA 1

ide_device ide_devices[4];
IDEChannelRegisters channels[2];
uint8_t ide_devices_count;

bool ide_0x1F0_controller_present = 0;
bool ide_0x170_controller_present = 0;
uint16_t ide_drive_type;
uint32_t ide_drive_size;

volatile static uint8_t ide_irq_invoked = 0;
static uint8_t atapi_packet[12] = {0xA8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
uint8_t package[1] = {0};

void ide_write(uint8_t channel, uint8_t reg, uint8_t data)
{
    // log_debug(MODULE, "enter ide_write(channel: %x, reg: %x, data: %x) file %s:%u", channel, reg, data, __FILE__, __LINE__);
    if (reg > 0x07 && reg < 0x0C)
        ide_write(channel, ATA_REG_CONTROL, 0x80 | channels[channel].nIEN);
    if (reg < 0x08)
        i686_outb(channels[channel].base + reg - 0x00, data);
    else if (reg < 0x0C)
        i686_outb(channels[channel].base + reg - 0x06, data);
    else if (reg < 0x0E)
        i686_outb(channels[channel].ctrl + reg - 0x0A, data);
    else if (reg < 0x16)
        i686_outb(channels[channel].bmide + reg - 0x0E, data);
    if (reg > 0x07 && reg < 0x0C)
        ide_write(channel, ATA_REG_CONTROL, channels[channel].nIEN);
    // log_debug(MODULE, "exit ide_write(channel: %x, reg: %x, data: %x) file %s:%u", channel, reg, data, __FILE__, __LINE__);
}

uint8_t ide_read(uint8_t channel, uint8_t reg)
{
    // log_debug(MODULE, "enter ide_read(channel: %x, reg: %x) file %s:%u", channel, reg, __FILE__, __LINE__);
    uint8_t result;
    if (reg > 0x07 && reg < 0x0C)
        ide_write(channel, ATA_REG_CONTROL, 0x80 | channels[channel].nIEN);
    if (reg < 0x08)
        result = i686_inb(channels[channel].base + reg - 0x00);
    else if (reg < 0x0C)
        result = i686_inb(channels[channel].base + reg - 0x06);
    else if (reg < 0x0E)
        result = i686_inb(channels[channel].ctrl + reg - 0x0A);
    else if (reg < 0x16)
        result = i686_inb(channels[channel].bmide + reg - 0x0E);
    if (reg > 0x07 && reg < 0x0C)
        ide_write(channel, ATA_REG_CONTROL, channels[channel].nIEN);
    // log_debug(MODULE, "exit ide_read(channel: %x, reg: %x) = %x file %s:%u", channel, reg, result, __FILE__, __LINE__);
    return result;
}

void ide_read_buffer(uint8_t channel, uint8_t reg, uint32_t buffer, uint32_t quads)
{
    // log_debug(MODULE, "enter ide_read_buffer(channel: %x, reg: %x, buffer: %x, quads: %x) file %s:%u", channel, reg, buffer, quads, __FILE__, __LINE__);
    /* WARNING: This code contains a serious bug. The inline assembly trashes ES and
     *           ESP for all of the code the compiler generates between the inline
     *           assembly blocks.
     */
    if (reg > 0x07 && reg < 0x0C)
    {
        ide_write(channel, ATA_REG_CONTROL, 0x80 | channels[channel].nIEN);
    }
    __asm__ volatile(
        "pushw %%es\n"      // Save ES
        "movw %%ds, %%ax\n" // Move DS to AX
        "movw %%ax, %%es\n" // Move AX to ES
        :
        :
        : "ax", "memory" // Clobbers: AX is modified, memory might be affected
    );
    if (reg < 0x08)
    {
        uint32_t port = channels[channel].base + reg - 0x00;
        insl(port, buffer, quads);
    }
    else if (reg < 0x0C)
    {
        insl(channels[channel].base + reg - 0x06, buffer, quads);
    }
    else if (reg < 0x0E)
    {
        insl(channels[channel].ctrl + reg - 0x0A, buffer, quads);
    }
    else if (reg < 0x16)
    {
        insl(channels[channel].bmide + reg - 0x0E, buffer, quads);
    }
    __asm__ volatile(
        "popw %%es\n" // Restore ES (if needed)
        :
        :
        : "memory");

    if (reg > 0x07 && reg < 0x0C)
    {
        ide_write(channel, ATA_REG_CONTROL, channels[channel].nIEN);
    }
    // log_debug(MODULE, "exit ide_read_buffer(channel: %x, reg: %x, buffer: %x, quads: %x) file %s:%u", channel, reg, buffer, quads, __FILE__, __LINE__);
    return;
}

void ide_400ns_delay(uint8_t channel)
{
    // log_debug(MODULE, "enter ide_400ns_delay(channel: %x) file %s:%u", channel, __FILE__, __LINE__);
    for (int i = 0; i < 4; i++)
        i686_inb(channel + ATA_REG_ALTSTATUS); // Reading the Alternate Status port wastes 100ns; loop four times.
}

uint8_t ide_polling(uint8_t channel, uint32_t advanced_check)
{
    // log_debug(MODULE, "enter ide_polling(channel: %x, advanced_check: %x) file %s:%u", channel, advanced_check, __FILE__, __LINE__);
    
    // (I) Delay 400 nanosecond for BSY to be set:
    // -------------------------------------------------
    ide_400ns_delay(channel);
    
    // (II) Wait for BSY to be cleared:
    // -------------------------------------------------
    while (ide_read(channel, ATA_REG_STATUS) & ATA_SR_BSY)
    {
        ; // Wait for BSY to be zero.
    }
    
    if (advanced_check)
    {
        uint8_t state = ide_read(channel, ATA_REG_STATUS); // Read Status Register.
        
        // (III) Check For Errors:
        // -------------------------------------------------
        if (state & ATA_SR_ERR)
        {
            log_debug(MODULE, "exit ide_polling(channel: %x, advanced_check: %x) with error 2 file %s:%u", channel, advanced_check, __FILE__, __LINE__);
            return 2; // Error.
        }
        
        // (IV) Check If Device fault:
        // -------------------------------------------------
        if (state & ATA_SR_DF)
        {
            log_debug(MODULE, "exit ide_polling(channel: %x, advanced_check: %x) with error 1 file %s:%u", channel, advanced_check, __FILE__, __LINE__);
            return 1; // Device Fault.
        }
        
        // (V) Check DRQ:
        // -------------------------------------------------
        // BSY = 0; DF = 0; ERR = 0 so we should check for DRQ now.
        if ((state & ATA_SR_DRQ) == 0)
        {
            log_debug(MODULE, "exit ide_polling(channel: %x, advanced_check: %x) with error 3 file %s:%u", channel, advanced_check, __FILE__, __LINE__);
            return 3; // DRQ should be set
        }
    }
    
    // log_debug(MODULE, "exit ide_polling(channel: %x, advanced_check: %x) with 0 file %s:%u", channel, advanced_check, __FILE__, __LINE__);
    return 0; // No Error.
}

uint8_t ide_print_error(uint32_t drive, uint8_t err)
{
    if (err == 0)
        return err;

    puts("IDE:");
    if (err == 1)
    {
        puts("- Device Fault\n     ");
        err = 19;
    }
    else if (err == 2)
    {
        uint8_t st = ide_read(ide_devices[drive].Channel, ATA_REG_ERROR);
        if (st & ATA_ER_AMNF)
        {
            puts("- No Address Mark Found\n     ");
            err = 7;
        }
        if (st & ATA_ER_TK0NF)
        {
            puts("- No Media or Media Error\n     ");
            err = 3;
        }
        if (st & ATA_ER_ABRT)
        {
            puts("- Command Aborted\n     ");
            err = 20;
        }
        if (st & ATA_ER_MCR)
        {
            puts("- No Media or Media Error\n     ");
            err = 3;
        }
        if (st & ATA_ER_IDNF)
        {
            puts("- ID mark not Found\n     ");
            err = 21;
        }
        if (st & ATA_ER_MC)
        {
            puts("- No Media or Media Error\n     ");
            err = 3;
        }
        if (st & ATA_ER_UNC)
        {
            puts("- Uncorrectable Data Error\n     ");
            err = 22;
        }
        if (st & ATA_ER_BBK)
        {
            puts("- Bad Sectors\n     ");
            err = 13;
        }
    }
    else if (err == 3)
    {
        puts("- Reads Nothing\n     ");
        err = 23;
    }
    else if (err == 4)
    {
        puts("- Write Protected\n     ");
        err = 8;
    }
    printf("- [%s %s] %s\n",
           ide_devices[drive].Channel == 0 ? "Primary" : "Secondary", // Use the channel as an index into the array
           ide_devices[drive].Drive == 0 ? "Master" : "Slave",        // Same as above, using the drive
           ide_devices[drive].Model);

    return err;
}

void ideSelectDrive(uint8_t channel, uint8_t drive)
{
    ide_write(channel, ATA_REG_HDDEVSEL, 0xA0 | (drive << 4)); // Select Drive.
    sleep_ms(1); // Wait 1ms for drive select to work.
}

bool ide_initialize(uint32_t BAR0, uint32_t BAR1, uint32_t BAR2, uint32_t BAR3, uint32_t BAR4)
{
    int j, i, k, count = 0;

    uint8_t ide_buf[0x200];

    // 1- Detect I/O Ports which interface IDE Controller:
    channels[ATA_PRIMARY].base = (BAR0 & 0xFFFFFFFC) + 0x1F0 * (!BAR0);
    channels[ATA_PRIMARY].ctrl = (BAR1 & 0xFFFFFFFC) + 0x3F6 * (!BAR1);
    channels[ATA_SECONDARY].base = (BAR2 & 0xFFFFFFFC) + 0x170 * (!BAR2);
    channels[ATA_SECONDARY].ctrl = (BAR3 & 0xFFFFFFFC) + 0x376 * (!BAR3);
    channels[ATA_PRIMARY].bmide = (BAR4 & 0xFFFFFFFC) + 0;   // Bus Master IDE
    channels[ATA_SECONDARY].bmide = (BAR4 & 0xFFFFFFFC) + 8; // Bus Master IDE

    // 2- Disable IRQs:
    ide_write(ATA_PRIMARY, ATA_REG_CONTROL, 2);
    ide_write(ATA_SECONDARY, ATA_REG_CONTROL, 2);

    // 3- Detect ATA-ATAPI Devices:
    for (i = 0; i < 2; i++)
    {
        // log_debug(MODULE, "start for 1");
        for (j = 0; j < 2; j++)
        {
            // log_debug(MODULE, "start for 2 channel = %u drive = %u", i, j << 4);
            uint8_t err = 0, type = IDE_ATA, status;
            ide_devices[count].Reserved = 0; // Assuming that no drive here.

            // (I) Select Drive:
            ideSelectDrive(i, j); // Select Drive.

            // (II) Send ATA Identify Command:
            ide_write(i, ATA_REG_COMMAND, ATA_CMD_IDENTIFY);
            sleep_ms(1); // This function should be implemented in your OS. which waits for 1 ms.
            //  it is based on System Timer Device Driver.

            // (III) Polling:
            // log_debug(MODULE, "polling");
            if (ide_read(i, ATA_REG_STATUS) == 0)
            {
                continue; // If Status = 0, No Device.
            }
            // log_debug(MODULE, "after polling");

            while (1)
            {
                status = ide_read(i, ATA_REG_STATUS);
                if ((status & ATA_SR_ERR))
                {
                    err = 1;
                    break;
                } // If Err, Device is not ATA.
                if (!(status & ATA_SR_BSY) && (status & ATA_SR_DRQ))
                {
                    // log_debug(MODULE, "good to go");
                    break; // Everything is right.
                }
                // log_debug(MODULE, "status = %u", status);
            }

            // (IV) Probe for ATAPI Devices:

            if (err != 0)
            {
                // log_crit(MODULE, "SOme Error %X", ide_read(i, ATA_SR_ERR));
                uint8_t cl = ide_read(i, ATA_REG_LBA1);
                uint8_t ch = ide_read(i, ATA_REG_LBA2);

                if (cl == 0x14 && ch == 0xEB)
                {
                    type = IDE_ATAPI;
                    // log_debug(MODULE, "1: Type = %u", type);
                }
                else if (cl == 0x69 && ch == 0x96)
                {
                    type = IDE_ATAPI;
                    // log_debug(MODULE, "2: Type = %u", type);
                }
                else
                {
                    // log_debug(MODULE, "Unknown Type");
                    continue; // Unknown Type (may not be a device).
                }

                ide_write(i, ATA_REG_COMMAND, ATA_CMD_IDENTIFY_PACKET);
                sleep_ms(1);
            }
            // (V) Read Identification Space of the Device:
            // log_debug(MODULE, "read buffer");
            ide_read_buffer(i, ATA_REG_DATA, (uint32_t)ide_buf, 128);
            // log_debug(MODULE, "after read buffer");

            // (VI) Read Device Parameters:
            ide_devices[count].Reserved = 1;
            ide_devices[count].Type = type;
            ide_devices[count].Channel = i;
            ide_devices[count].Drive = j;
            ide_devices[count].Signature = *((uint16_t *)(ide_buf + ATA_IDENT_DEVICETYPE));
            ide_devices[count].Capabilities = *((uint16_t *)(ide_buf + ATA_IDENT_CAPABILITIES));
            ide_devices[count].CommandSets = *((uint32_t *)(ide_buf + ATA_IDENT_COMMANDSETS));

            // log_debug(MODULE, "Read Device Parameters");

            // (VII) Get Size:
            if (ide_devices[count].CommandSets & (1 << 26))
            {
                // Device uses 48-Bit Addressing:
                ide_devices[count].Size = *((uint32_t *)(ide_buf + ATA_IDENT_MAX_LBA_EXT));
            }
            else
            {
                // Device uses CHS or 28-bit Addressing:
                ide_devices[count].Size = *((uint32_t *)(ide_buf + ATA_IDENT_MAX_LBA));
            }
            // log_debug(MODULE, "Get Size %u", ide_devices[count].Size);

            // (VIII) String indicates model of device (like Western Digital HDD and SONY DVD-RW...):
            for (k = 0; k < 40; k += 2)
            {
                ide_devices[count].Model[k] = ide_buf[ATA_IDENT_MODEL + k + 1];
                ide_devices[count].Model[k + 1] = ide_buf[ATA_IDENT_MODEL + k];
            }
            ide_devices[count].Model[40] = 0; // Terminate String.
            ide_devices_count++;

            count++;
        }
    }

    // log_debug(MODULE, "Print Summary");
    // 4- Print Summary:
    int foundDrives = 0;
    for (i = 0; i < 4; i++)
    {
        if (ide_devices[i].Reserved == 1)
        {
            foundDrives++;
            printf(" Found %s Drive %dGB - %s\n",
                   (const char *[]){"ATA", "ATAPI"}[ide_devices[i].Type], /* Type */
                   ide_devices[i].Size / 1024 / 1024 / 2,                 /* Size */
                   ide_devices[i].Model);
        }
    }
    if (foundDrives == 0)
    {
        return false;
    }
    return true;
}

uint8_t GetCmd(uint8_t lba_mode, uint8_t dma, uint8_t direction)
{
    log_debug(MODULE, "file %s:%u", __FILE__, __LINE__);
    if (dma == NoDMA)
    {
        if (direction == ATA_READ)
        {
            switch (lba_mode)
            {
            case CHS:
            case LBA28:
                return ATA_CMD_READ_PIO;
            case LBA48:
                return ATA_CMD_READ_PIO_EXT;

            default:
                break;
            }
        }
        else if (direction == ATA_WRITE)
        {
            switch (lba_mode)
            {
            case CHS:
            case LBA28:
                return ATA_CMD_WRITE_PIO;
            case LBA48:
                return ATA_CMD_WRITE_PIO_EXT;

            default:
                break;
            }
        }
    }
    else if (dma == HasDMA)
    {
        if (direction == ATA_READ)
        {
            switch (lba_mode)
            {
            case CHS:
            case LBA28:
                return ATA_CMD_READ_DMA;
            case LBA48:
                return ATA_CMD_READ_DMA_EXT;

            default:
                break;
            }
        }
        else if (direction == ATA_WRITE)
        {
            switch (lba_mode)
            {
            case CHS:
            case LBA28:
                return ATA_CMD_WRITE_DMA;
            case LBA48:
                return ATA_CMD_WRITE_DMA_EXT;

            default:
                break;
            }
        }
    }
    return false;
}

uint8_t ide_ata_access(uint8_t direction, uint8_t drive, uint32_t lba, uint8_t numsects, uint16_t selector, uint32_t edi)
{
    log_debug(MODULE, "file %s:%u", __FILE__, __LINE__);
    uint8_t lba_mode /* 0: CHS, 1:LBA28, 2: LBA48 */, dma /* 0: No DMA, 1: DMA */, cmd;
    uint8_t lba_io[6];
    uint32_t channel = ide_devices[drive].Channel; // Read the Channel.
    uint32_t slavebit = ide_devices[drive].Drive;  // Read the Drive [Master/Slave]
    uint32_t bus = channels[channel].base;         // Bus Base, like 0x1F0 which is also data port.
    uint32_t words = 256;                          // Almost every ATA drive has a sector-size of 512-byte.
    uint16_t cyl, i;
    uint8_t head, sect, err;

    ide_write(channel, ATA_REG_CONTROL, channels[channel].nIEN = (ide_irq_invoked = 0x0) + 0x02);

    // (I) Select one from LBA28, LBA48 or CHS;
    if (lba >= 0x10000000)
    { // Sure Drive should support LBA in this case, or you are
        // giving a wrong LBA.
        // LBA48:
        lba_mode = LBA48;
        lba_io[0] = (lba & 0x000000FF) >> 0;
        lba_io[1] = (lba & 0x0000FF00) >> 8;
        lba_io[2] = (lba & 0x00FF0000) >> 16;
        lba_io[3] = (lba & 0xFF000000) >> 24;
        lba_io[4] = 0; // LBA28 is integer, so 32-bits are enough to access 2TB.
        lba_io[5] = 0; // LBA28 is integer, so 32-bits are enough to access 2TB.
        head = 0;      // Lower 4-bits of HDDEVSEL are not used here.
    }
    else if (ide_devices[drive].Capabilities & 0x200)
    { // Drive supports LBA?
        // LBA28:
        lba_mode = LBA28;
        lba_io[0] = (lba & 0x00000FF) >> 0;
        lba_io[1] = (lba & 0x000FF00) >> 8;
        lba_io[2] = (lba & 0x0FF0000) >> 16;
        lba_io[3] = 0; // These Registers are not used here.
        lba_io[4] = 0; // These Registers are not used here.
        lba_io[5] = 0; // These Registers are not used here.
        head = (lba & 0xF000000) >> 24;
    }
    else
    {
        // CHS:
        lba_mode = CHS;
        sect = (lba % 63) + 1;
        cyl = (lba + 1 - sect) / (16 * 63);
        lba_io[0] = sect;
        lba_io[1] = (cyl >> 0) & 0xFF;
        lba_io[2] = (cyl >> 8) & 0xFF;
        lba_io[3] = 0;
        lba_io[4] = 0;
        lba_io[5] = 0;
        head = (lba + 1 - sect) % (16 * 63) / (63); // Head number is written to HDDEVSEL lower 4-bits.
    }

    // (II) See if drive supports DMA or not;
    dma = NoDMA; // We don't support DMA

    // (III) Wait if the drive is busy;
    while (ide_read(channel, ATA_REG_STATUS) & ATA_SR_BSY)
    {

    } // Wait if busy.

    // (IV) Select Drive from the controller;
    if (lba_mode == CHS)
    {
        ide_write(channel, ATA_REG_HDDEVSEL, 0xA0 | (slavebit << 4) | head); // Drive & CHS.
    }
    else
    {
        ide_write(channel, ATA_REG_HDDEVSEL, 0xE0 | (slavebit << 4) | head); // Drive & LBA
    }

    // (V) Write Parameters;
    if (lba_mode == LBA48)
    {
        ide_write(channel, ATA_REG_SECCOUNT1, 0);
        ide_write(channel, ATA_REG_LBA3, lba_io[3]);
        ide_write(channel, ATA_REG_LBA4, lba_io[4]);
        ide_write(channel, ATA_REG_LBA5, lba_io[5]);
    }
    ide_write(channel, ATA_REG_SECCOUNT0, numsects);
    ide_write(channel, ATA_REG_LBA0, lba_io[0]);
    ide_write(channel, ATA_REG_LBA1, lba_io[1]);
    ide_write(channel, ATA_REG_LBA2, lba_io[2]);

    // (VI) Select the command and send it;
    // Routine that is followed:
    // If ( DMA & LBA48)   DO_DMA_EXT;
    // If ( DMA & LBA28)   DO_DMA_LBA;
    // If ( DMA & LBA28)   DO_DMA_CHS;
    // If (!DMA & LBA48)   DO_PIO_EXT;
    // If (!DMA & LBA28)   DO_PIO_LBA;
    // If (!DMA & !LBA#)   DO_PIO_CHS;

    cmd = GetCmd(lba_mode, dma, direction);
    ide_write(channel, ATA_REG_COMMAND, cmd); // Send the Command.

    if (dma)
        if (direction == 0)
            ;
        // DMA Read.
        else
            ;
    // DMA Write.
    else if (direction == 0)
        // PIO Read.
        for (i = 0; i < numsects; i++)
        {
            err = ide_polling(channel, 1);
            if (err)
                return err; // Polling, set error and exit if there is.
            ReceiveData(bus, selector, words, edi);

            edi += (words * 2);
        }
    else
    {
        // PIO Write.
        for (i = 0; i < numsects; i++)
        {
            ide_polling(channel, 0); // Polling.
            // Send data via outsw
            SendData(bus, selector, words, edi);
            edi += (words * 2);
        }
        ide_write(channel, ATA_REG_COMMAND, (char[]){ATA_CMD_CACHE_FLUSH, ATA_CMD_CACHE_FLUSH, ATA_CMD_CACHE_FLUSH_EXT}[lba_mode]);
        ide_polling(channel, 0); // Polling.
    }
    log_debug(MODULE, "exit file %s:%u", __FILE__, __LINE__);
    return 0; // Easy, isn't it?
}

void ide_wait_irq()
{
    log_debug(MODULE, "file %s:%u", __FILE__, __LINE__);
    while (!ide_irq_invoked)
        ;
    ide_irq_invoked = 0;
}
void ide_irq()
{
    log_debug(MODULE, "file %s:%u", __FILE__, __LINE__);
    ide_irq_invoked = 1;
}

uint8_t ide_atapi_read(uint8_t drive, uint32_t lba, uint8_t numsects, uint16_t selector, uint32_t edi)
{
    log_debug(MODULE, "file %s:%u", __FILE__, __LINE__);
    uint8_t channel = ide_devices[drive].Channel;
    uint32_t slavebit = ide_devices[drive].Drive;
    uint32_t bus = channels[channel].base;
    uint32_t words = 1024; // Sector Size. ATAPI drives have a sector size of 2048 bytes.
    uint8_t err;
    int i;

    // Enable IRQs:
    ide_write(channel, ATA_REG_CONTROL, channels[channel].nIEN = ide_irq_invoked = 0x0);

    // (I): Setup SCSI Packet:
    // ------------------------------------------------------------------
    atapi_packet[0] = ATAPI_CMD_READ;
    atapi_packet[1] = 0x0;
    atapi_packet[2] = (lba >> 24) & 0xFF;
    atapi_packet[3] = (lba >> 16) & 0xFF;
    atapi_packet[4] = (lba >> 8) & 0xFF;
    atapi_packet[5] = (lba >> 0) & 0xFF;
    atapi_packet[6] = 0x0;
    atapi_packet[7] = 0x0;
    atapi_packet[8] = 0x0;
    atapi_packet[9] = numsects;
    atapi_packet[10] = 0x0;
    atapi_packet[11] = 0x0;

    // (II): Select the drive:
    // ------------------------------------------------------------------
    ide_write(channel, ATA_REG_HDDEVSEL, slavebit << 4);

    // (III): Delay 400 nanoseconds for select to complete:
    // ------------------------------------------------------------------
    ide_400ns_delay(channel);

    // (IV): Inform the Controller that we use PIO mode:
    // ------------------------------------------------------------------
    ide_write(channel, ATA_REG_FEATURES, 0); // PIO mode.

    // (V): Tell the Controller the size of buffer:
    // ------------------------------------------------------------------
    ide_write(channel, ATA_REG_LBA1, (words * 2) & 0xFF); // Lower Byte of Sector Size.
    ide_write(channel, ATA_REG_LBA2, (words * 2) >> 8);   // Upper Byte of Sector Size.

    // (VI): Send the Packet Command:
    // ------------------------------------------------------------------
    ide_write(channel, ATA_REG_COMMAND, ATA_CMD_PACKET); // Send the Command.

    // (VII): Waiting for the driver to finish or return an error code:
    // ------------------------------------------------------------------
    err = ide_polling(channel, 1);
    if (err)
        return err; // Polling and return if error.

    // (VIII): Sending the packet data:
    // ------------------------------------------------------------------
    SendPacket(bus, atapi_packet);

    // (IX): Receiving Data:
    // ------------------------------------------------------------------
    for (i = 0; i < numsects; i++)
    {
        ide_wait_irq(); // Wait for an IRQ.
        err = ide_polling(channel, 1);
        if (err)
            return err; // Polling and return if error.
        // Receive data via insw
        // selector, words, bus, edi
        ReceiveData(selector, words, bus, edi);
        edi += (words * 2);
    }

    // (X): Waiting for an IRQ:
    // ------------------------------------------------------------------
    ide_wait_irq();

    // (XI): Waiting for BSY & DRQ to clear:
    // ------------------------------------------------------------------
    while (ide_read(channel, ATA_REG_STATUS) & (ATA_SR_BSY | ATA_SR_DRQ))
        ;
    log_debug(MODULE, "exit file %s:%u", __FILE__, __LINE__);
    return 0; // Easy, ... Isn't it?
}

void ide_read_sectors(uint8_t drive, uint8_t numsects, uint32_t lba, uint16_t es, uint32_t edi)
{
    log_debug(MODULE, "file %s:%u", __FILE__, __LINE__);
    // 1: Check if the drive presents:
    // ==================================
    if (drive > 3 || ide_devices[drive].Reserved == 0)
    {
        package[0] = 0x1; // Drive Not Found!
    }

    // 2: Check if inputs are valid:
    // ==================================
    else if (((lba + numsects) > ide_devices[drive].Size) && (ide_devices[drive].Type == IDE_ATA))
    {
        package[0] = 0x2; // Seeking to invalid position.
    }

    // 3: Read in PIO Mode through Polling & IRQs:
    // ============================================
    else
    {
        uint8_t err;
        if (ide_devices[drive].Type == IDE_ATA)
        {
            err = ide_ata_access(ATA_READ, drive, lba, numsects, es, edi);
        }
        else if (ide_devices[drive].Type == IDE_ATAPI)
        {
            for (int i = 0; i < numsects; i++)
            {
                err = ide_atapi_read(drive, lba + i, 1, es, edi + (i * 2048));
            }
        }
        package[0] = ide_print_error(drive, err);
    }
    log_debug(MODULE, "exit file %s:%u", __FILE__, __LINE__);
}

void ide_write_sectors(uint8_t drive, uint8_t numsects, uint32_t lba, uint16_t es, uint32_t edi)
{
    log_debug(MODULE, "file %s:%u", __FILE__, __LINE__);
    // 1: Check if the drive presents:
    // ==================================
    if (drive > 3 || ide_devices[drive].Reserved == 0)
    {
        package[0] = 0x1; // Drive Not Found!
    }
    // 2: Check if inputs are valid:
    // ==================================
    else if (((lba + numsects) > ide_devices[drive].Size) && (ide_devices[drive].Type == IDE_ATA))
    {
        package[0] = 0x2; // Seeking to invalid position.
    }
    // 3: Read in PIO Mode through Polling & IRQs:
    // ============================================
    else
    {
        uint8_t err;
        if (ide_devices[drive].Type == IDE_ATA)
        {
            err = ide_ata_access(ATA_WRITE, drive, lba, numsects, es, edi);
        }
        else if (ide_devices[drive].Type == IDE_ATAPI)
        {
            err = 4; // Write-Protected.
        }
        package[0] = ide_print_error(drive, err);
    }
    log_debug(MODULE, "exit file %s:%u", __FILE__, __LINE__);
}

uint8_t ide_is_bus_floating(uint16_t base_port)
{
    log_debug(MODULE, "file %s:%u", __FILE__, __LINE__);
    if (i686_inb(base_port + 7) == 0xFF || i686_inb(base_port + 7) == 0x7F)
    { // invalid state of status register
        return STATUS_TRUE;
    }
    else
    {
        return STATUS_FALSE;
    }
}
