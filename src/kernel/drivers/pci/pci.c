#include <stdint.h>

#include "stdio.h"
#include "string.h"
#include "debug.h"
#include "hal/hal.h"
#include "memory.h"
#include "malloc.h"
#include "arch/i686/io.h"
#include "arch/i686/i8259.h"
#include "drivers/ide/ide_controller.h"

#include "drivers/ATA/ATA.h"
#include "drivers/ATA/storage.h"

#include "pci.h"

#define MODULE "PCI"

pci_device **pci_devices = 0;
uint32_t devs = 0;

uint32_t pci_devices_array_mem;
uint32_t pci_num_of_devices;

pci_driver **pci_drivers = 0;
uint32_t drivs = 0;

#define getAddress(bus, device, function, offset) (uint64_t)(0x80000000 | (bus << 16) | (device << 11) | (function << 8) | offset);

void add_pci_device(pci_device *pdev)
{
    pci_devices[devs] = pdev;
    devs++;
    return;
}

uint32_t pci_read_int32(uint32_t bus, uint32_t device, uint32_t function, uint32_t offset)
{
    uint64_t address = getAddress(bus, device, function, offset);
    i686_outd(0xCF8, address);
    return i686_ind(0xCFC);
}

uint16_t pci_read_int16(uint32_t bus, uint32_t device, uint32_t function, uint32_t offset)
{
    uint16_t tmp = 0;
    uint64_t address = getAddress(bus, device, function, offset);
    i686_outd(0xCF8, address);
    tmp = (uint16_t)((i686_ind(0xCFC) >> ((offset & 2) * 8)) & 0xffff);
    return (tmp);
}

void pci_write(uint32_t bus, uint32_t device, uint32_t function, uint32_t offset, uint32_t value)
{
    uint64_t address = getAddress(bus, device, function, offset);
    i686_outd(0xCF8, address);
    i686_outd(0xCFC, value);
}

uint32_t pci_read_bar_type(uint32_t bus, uint32_t device, uint32_t function, uint32_t bar)
{
    uint64_t address = getAddress(bus, device, function, bar);
    i686_outd(0xCF8, address);
    return (uint16_t)(i686_ind(0xCFC) & 0x1);
}

uint16_t pci_read_io_bar(uint32_t bus, uint32_t device, uint32_t function, uint32_t bar)
{
    uint64_t address = getAddress(bus, device, function, bar);
    i686_outd(0xCF8, address);
    return (uint16_t)(i686_ind(0xCFC) & 0xFFFC);
}

uint32_t pci_read_mmio_bar(uint32_t bus, uint32_t device, uint32_t function, uint32_t bar)
{
    uint64_t address = getAddress(bus, device, function, bar);
    i686_outd(0xCF8, address);
    return (i686_ind(0xCFC) & 0xFFFFFFF0);
}

void pci_enable_io_busmastering(uint32_t bus, uint32_t device, uint32_t function)
{
    pci_write(bus, device, function, 0x04, ((pci_read_int32(bus, device, function, 0x04) & ~(1 << 10)) | (1 << 2) | (1 << 0))); // enable interrupts, enable bus mastering, enable IO space
}

void pci_enable_mmio_busmastering(uint32_t bus, uint32_t device, uint32_t function)
{
    pci_write(bus, device, function, 0x04, ((pci_read_int32(bus, device, function, 0x04) & ~(1 << 10)) | (1 << 2) | (1 << 1))); // enable interrupts, enable bus mastering, enable MMIO space
}

void pci_disable_interrupts(uint32_t bus, uint32_t device, uint32_t function)
{
    pci_write(bus, device, function, 0x04, (pci_read_int32(bus, device, function, 0x04) | (1 << 10))); // disable interrupts
}

uint32_t getVendorID(uint32_t bus, uint32_t device, uint32_t function)
{
    uint32_t r0 = pci_read_int32(bus, device, function, 0);
    return r0;
}
uint32_t getDeviceID(uint32_t bus, uint32_t device, uint32_t function)
{
    uint32_t r0 = pci_read_int32(bus, device, function, 2);
    return r0;
}
uint16_t getClassId(uint16_t bus, uint16_t device, uint16_t function)
{
    uint32_t r0 = pci_read_int16(bus, device, function, 0xA);
    return (r0 & ~0x00FF) >> 8;
}
uint16_t getSubClassId(uint16_t bus, uint16_t device, uint16_t function)
{
    uint32_t r0 = pci_read_int16(bus, device, function, 0xA);
    return (r0 & ~0xFF00);
}

void scan_pci(void)
{
    // initalize values that are used to determine presence of devices
    ide_0x1F0_controller_present = STATUS_FALSE;
    ide_0x170_controller_present = STATUS_FALSE;
    // usb_controllers_pointer = 0;
    // number_of_graphic_cards = 0;
    // number_of_sound_cards = 0;
    number_of_storage_controllers = 0;
    // number_of_ethernet_cards = 0;

    // this array is used in System board
    pci_devices_array_mem = (uint32_t)calloc(12 * 1000, sizeof(uint32_t));
    pci_num_of_devices = 0;

    log_debug(MODULE "\n\nPCI devices:", 0);

    for (int bus = 0; bus < 256; bus++)
    {
        for (int device = 0; device < 32; device++)
        {
            scan_pci_device(bus, device, 0);

            // multifunctional device
            if ((pci_read_int32(bus, device, 0, 0x0C) & 0x00800000) == 0x00800000)
            {
                for (int function = 1; function < 8; function++)
                {
                    scan_pci_device(bus, device, function);
                }
            }
        }
    }
}

void scan_pci_device(uint32_t bus, uint32_t device, uint32_t function)
{
    uint32_t full_device_id, vendor_id, device_id, type_of_device, class, subclass, progif, device_irq, mmio_port_base;
    uint16_t io_port_base;

    // read base informations about device
    vendor_id = (getVendorID(bus, device, function) & 0xFFFF);
    device_id = (getVendorID(bus, device, function) >> 16);
    full_device_id = pci_read_int32(bus, device, function, 0);
    if (full_device_id == 0xFFFFFFFF)
    {
        return; // no device
    }
    type_of_device = (pci_read_int32(bus, device, function, 0x08) >> 8);
    class = (type_of_device >> 16);
    subclass = ((type_of_device >> 8) & 0xFF);
    progif = (type_of_device & 0xFF);
    device_irq = (pci_read_int32(bus, device, function, 0x3C) & 0xFF);

    // save device to array
    if (pci_num_of_devices < 1000)
    {
        uint32_t *pci_devices_array = (uint32_t *)(pci_devices_array_mem + pci_num_of_devices * 12);
        pci_devices_array[0] = (bus | (device << 8) | (function << 16));
        pci_devices_array[1] = type_of_device;
        pci_devices_array[2] = full_device_id;
        pci_num_of_devices++;
    }

    // Graphic card
    /*
    if(type_of_device==0x030000 && number_of_graphic_cards<MAX_NUMBER_OF_GRAPHIC_CARDS) {
     log_debug(MODULE, "\nGraphic card");

     if(number_of_graphic_cards>=MAX_NUMBER_OF_GRAPHIC_CARDS) {
      return;
     }

     graphic_cards_info[number_of_graphic_cards].vendor_id = vendor_id;
     graphic_cards_info[number_of_graphic_cards].device_id = device_id;

     if(vendor_id==VENDOR_INTEL) {
      pci_enable_io_busmastering(bus, device, function);
      graphic_cards_info[number_of_graphic_cards].mmio_base = pci_read_mmio_bar(bus, device, function, PCI_BAR0);
      graphic_cards_info[number_of_graphic_cards].linear_frame_buffer = (uint8_t *) (pci_read_mmio_bar(bus, device, function, PCI_BAR2));
     }

     number_of_graphic_cards++;
     return;
    }
     */

    // IDE controller
    if ((type_of_device & 0xFFFF00) == 0x010100 && number_of_storage_controllers < MAX_NUMBER_OF_STORAGE_CONTROLLERS)
    {
    pci_device_ide_controller:
        log_debug(MODULE, "\nIDE controller");
        pci_enable_io_busmastering(bus, device, function);
        pci_disable_interrupts(bus, device, function);

        // first controller
        storage_controllers[number_of_storage_controllers].controller_type = IDE_CONTROLLER;
        io_port_base = pci_read_io_bar(bus, device, function, PCI_BAR0);
        if (io_port_base == 0 || ide_is_bus_floating(io_port_base) == STATUS_FALSE)
        {
            if (io_port_base == 0 || io_port_base == 0x1F0)
            {
                if (get_ide_0x1F0_controller_present() == STATUS_FALSE)
                {
                    set_ide_0x1F0_controller_present(STATUS_TRUE);
                    storage_controllers[number_of_storage_controllers].base_1 = 0x1F0;
                    storage_controllers[number_of_storage_controllers].base_2 = 0x3F4;
                    number_of_storage_controllers++;
                }
            }
            else
            {
                storage_controllers[number_of_storage_controllers].base_1 = io_port_base;
                storage_controllers[number_of_storage_controllers].base_2 = pci_read_io_bar(bus, device, function, PCI_BAR1);
                number_of_storage_controllers++;
            }
        }
        if (number_of_storage_controllers >= MAX_NUMBER_OF_STORAGE_CONTROLLERS)
        {
            return;
        }

        // second controller
        storage_controllers[number_of_storage_controllers].controller_type = IDE_CONTROLLER;
        io_port_base = pci_read_io_bar(bus, device, function, PCI_BAR2);
        if (io_port_base == 0 || ide_is_bus_floating(io_port_base) == STATUS_FALSE)
        {
            if (io_port_base == 0 || io_port_base == 0x170)
            {
                if (get_ide_0x170_controller_present() == STATUS_FALSE)
                {
                    set_ide_0x170_controller_present(STATUS_TRUE);
                    storage_controllers[number_of_storage_controllers].base_1 = 0x170;
                    storage_controllers[number_of_storage_controllers].base_2 = 0x374;
                    number_of_storage_controllers++;
                }
            }
            else
            {
                storage_controllers[number_of_storage_controllers].base_1 = io_port_base;
                storage_controllers[number_of_storage_controllers].base_2 = pci_read_io_bar(bus, device, function, PCI_BAR3);
                number_of_storage_controllers++;
            }
        }

        return;
    }

    // SATA controller
    if (type_of_device == 0x010601 && number_of_storage_controllers < MAX_NUMBER_OF_STORAGE_CONTROLLERS)
    {
        log_debug(MODULE, "\nAHCI controller");
        pci_enable_mmio_busmastering(bus, device, function);
        pci_disable_interrupts(bus, device, function);

        // test presence of AHCI interface
        if ((i686_ind(pci_read_mmio_bar(bus, device, function, PCI_BAR5) + 0x04) & 0x80000000) == 0x00000000)
        {
            log_debug(MODULE, " with IDE interface");
            goto pci_device_ide_controller; // IDE interface
        }

        // save device
        storage_controllers[number_of_storage_controllers].controller_type = AHCI_CONTROLLER;
        storage_controllers[number_of_storage_controllers].base_1 = pci_read_mmio_bar(bus, device, function, PCI_BAR5);
        number_of_storage_controllers++;
        return;
    }

    /*
    // Ethernet card
    if (type_of_device == 0x020000)
    {
        log_debug(MODULE, "\nEthernet card ");
        log_hex_with_space(full_device_id);

        if (number_of_ethernet_cards >= MAX_NUMBER_OF_ETHERNET_CARDS)
        {
            return;
        }

        // read basic values
        ethernet_cards[number_of_ethernet_cards].id = full_device_id;
        ethernet_cards[number_of_ethernet_cards].irq = (pci_read(bus, device, function, 0x3C) & 0xFF);
        ethernet_cards[number_of_ethernet_cards].bus = bus;
        ethernet_cards[number_of_ethernet_cards].dev = device;
        ethernet_cards[number_of_ethernet_cards].func = function;
        ethernet_cards[number_of_ethernet_cards].initalize = 0; // by default card has no driver

        // Intel E1000
        if (vendor_id == VENDOR_INTEL)
        {
            // connect to driver for Intel e1000
            ethernet_cards[number_of_ethernet_cards].initalize = ec_intel_e1000_initalize;

            ethernet_cards[number_of_ethernet_cards].bar_type = pci_read_bar_type(bus, device, function, PCI_BAR0);
            if (ethernet_cards[number_of_ethernet_cards].bar_type == PCI_MMIO_BAR)
            {
                ethernet_cards[number_of_ethernet_cards].base = pci_read_mmio_bar(bus, device, function, PCI_BAR0);
                pci_enable_mmio_busmastering(bus, device, function);
            }
            else
            {
                ethernet_cards[number_of_ethernet_cards].base = pci_read_io_bar(bus, device, function, PCI_BAR0);
                pci_enable_io_busmastering(bus, device, function);
            }
        }

        // AMD PC-net
        else if (vendor_id == VENDOR_AMD_1 || vendor_id == VENDOR_AMD_2)
        {
            // connect to driver for Amd PCNET
            ethernet_cards[number_of_ethernet_cards].initalize = ec_amd_pcnet_initalize;

            pci_enable_io_busmastering(bus, device, function);
            ethernet_cards[number_of_ethernet_cards].bar_type = PCI_IO_BAR;
            ethernet_cards[number_of_ethernet_cards].base = pci_read_io_bar(bus, device, function, PCI_BAR0);
        }

        // Broadcom NetXtreme
        else if (vendor_id == VENDOR_BROADCOM)
        {
            // connect to driver for Broadcom NetXtreme
            ethernet_cards[number_of_ethernet_cards].initalize = ec_broadcom_netxtreme_initalize;

            pci_enable_mmio_busmastering(bus, device, function);
            ethernet_cards[number_of_ethernet_cards].bar_type = PCI_MMIO_BAR;
            ethernet_cards[number_of_ethernet_cards].base = pci_read_mmio_bar(bus, device, function, PCI_BAR0);
        }

        // Qualcomm Atheros
        else if (vendor_id == VENDOR_QUALCOMM_ATHEROS_1 || vendor_id == VENDOR_QUALCOMM_ATHEROS_2)
        {
            // connect to driver for Qualcomm Atheros
            ethernet_cards[number_of_ethernet_cards].initalize = ec_atheros_initalize;

            pci_enable_io_busmastering(bus, device, function);
            pci_enable_mmio_busmastering(bus, device, function);
            ethernet_cards[number_of_ethernet_cards].bar_type = PCI_MMIO_BAR;
            ethernet_cards[number_of_ethernet_cards].base = pci_read_mmio_bar(bus, device, function, PCI_BAR0);
        }

        // Realtek
        else if (vendor_id == VENDOR_REALTEK)
        {
            if (device_id == 0x8139)
            {
                // connect to driver for Realtek 8139
                ethernet_cards[number_of_ethernet_cards].initalize = ec_realtek_8139_initalize;

                pci_enable_io_busmastering(bus, device, function);
                ethernet_cards[number_of_ethernet_cards].bar_type = PCI_IO_BAR;
                ethernet_cards[number_of_ethernet_cards].base = pci_read_io_bar(bus, device, function, PCI_BAR0);
            }
            else if (device_id == 0x8136 || device_id == 0x8161 || device_id == 0x8168 || device_id == 0x8169)
            {
                // connect to driver for Realtek 8169
                ethernet_cards[number_of_ethernet_cards].initalize = ec_realtek_8169_initalize;

                ethernet_cards[number_of_ethernet_cards].bar_type = pci_read_bar_type(bus, device, function, PCI_BAR0);
                if (ethernet_cards[number_of_ethernet_cards].bar_type == PCI_MMIO_BAR)
                {
                    ethernet_cards[number_of_ethernet_cards].base = pci_read_mmio_bar(bus, device, function, PCI_BAR0);
                    pci_enable_mmio_busmastering(bus, device, function);
                }
                else
                {
                    ethernet_cards[number_of_ethernet_cards].base = pci_read_io_bar(bus, device, function, PCI_BAR0);
                    pci_enable_io_busmastering(bus, device, function);
                }
            }
        }

        number_of_ethernet_cards++;
        return;
    }
    */

    /*
    // Ethernet card
    if (type_of_device == 0x028000)
    {
        log_debug(MODULE, "\nWireless card ");
        log_hex_with_space(full_device_id);

        return;
    }

    // AC97 sound card
    if (type_of_device == 0x040100 && number_of_sound_cards < MAX_NUMBER_OF_SOUND_CARDS)
    {
        log_debug(MODULE, "\nsound card AC97");

        if (number_of_sound_cards >= MAX_NUMBER_OF_SOUND_CARDS)
        {
            return;
        }

        pci_enable_io_busmastering(bus, device, function);

        sound_cards_info[number_of_sound_cards].driver = SOUND_CARD_DRIVER_AC97;
        sound_cards_info[number_of_sound_cards].vendor_id = vendor_id;
        sound_cards_info[number_of_sound_cards].device_id = device_id;
        sound_cards_info[number_of_sound_cards].io_base = pci_read_io_bar(bus, device, function, PCI_BAR0);
        sound_cards_info[number_of_sound_cards].io_base_2 = pci_read_io_bar(bus, device, function, PCI_BAR1);
        number_of_sound_cards++;

        return;
    }

    // HD Audio sound card
    if (type_of_device == 0x040300 && number_of_sound_cards < MAX_NUMBER_OF_SOUND_CARDS)
    {
        log_debug(MODULE, "\nsound card HDA");

        if (number_of_sound_cards >= MAX_NUMBER_OF_SOUND_CARDS)
        {
            return;
        }

        pci_enable_mmio_busmastering(bus, device, function);

        sound_cards_info[number_of_sound_cards].driver = SOUND_CARD_DRIVER_HDA;
        sound_cards_info[number_of_sound_cards].vendor_id = vendor_id;
        sound_cards_info[number_of_sound_cards].device_id = device_id;
        sound_cards_info[number_of_sound_cards].mmio_base = pci_read_mmio_bar(bus, device, function, PCI_BAR0);
        number_of_sound_cards++;

        return;
    }

    // Universal Host Controller Interface
    if (type_of_device == 0x0C0300)
    {
        log_debug(MODULE, "\nUHCI");

        if (usb_controllers_pointer >= MAX_NUMBER_OF_USB_CONTROLLERS)
        {
            return;
        }

        usb_controllers[usb_controllers_pointer].type = USB_CONTROLLER_UHCI;
        usb_controllers[usb_controllers_pointer].base = pci_read_io_bar(bus, device, function, PCI_BAR4);
        usb_controllers_pointer++;

        // disable BIOS legacy support
        pci_write(bus, device, function, 0xC0, 0x8F00);

        return;
    }

    // Open Host Controller Interface
    if (type_of_device == 0x0C0310)
    {
        log_debug(MODULE, "\nOHCI");

        if (usb_controllers_pointer >= MAX_NUMBER_OF_USB_CONTROLLERS)
        {
            return;
        }

        usb_controllers[usb_controllers_pointer].type = USB_CONTROLLER_OHCI;
        usb_controllers[usb_controllers_pointer].base = pci_read_mmio_bar(bus, device, function, PCI_BAR0);
        usb_controllers_pointer++;

        // disable BIOS legacy support
        if (mmio_ind(usb_controllers[usb_controllers_pointer].base + 0x0) == 0x110)
        {
            mmio_outd(usb_controllers[usb_controllers_pointer].base + 0x100, 0x00);
        }

        return;
    }

    // Enhanced Host Controller Interface
    if (type_of_device == 0x0C0320)
    {
        log_debug(MODULE, "\nEHCI");

        if (usb_controllers_pointer >= MAX_NUMBER_OF_USB_CONTROLLERS)
        {
            return;
        }

        usb_controllers[usb_controllers_pointer].bus = bus;
        usb_controllers[usb_controllers_pointer].device = device;
        usb_controllers[usb_controllers_pointer].function = function;
        usb_controllers[usb_controllers_pointer].type = USB_CONTROLLER_EHCI;
        usb_controllers[usb_controllers_pointer].base = pci_read_mmio_bar(bus, device, function, PCI_BAR0);
        usb_controllers_pointer++;

        return;
    }
    */

    // eXtensible Host Controller Interface
    if (type_of_device == 0x0C0330)
    {
        log_debug(MODULE, "\nxHCI");

        return;
    }

    // Host bridge
    if (type_of_device == 0x060000)
    {
        log_debug(MODULE, "\nHost bridge");

        return;
    }

    // ISA bridge
    if (type_of_device == 0x060100)
    {
        log_debug(MODULE, "\nISA bridge");

        return;
    }

    // PCI bridge
    if ((type_of_device & 0xFFFF00) == 0x060400)
    {
        log_debug(MODULE, "\nPCI bridge");

        return;
    }

    // Other bridge
    if (type_of_device == 0x068000)
    {
        log_debug(MODULE, "\nOther bridge");

        return;
    }

    log_debug(MODULE, "\nunknown device %X", type_of_device);
    pci_disable_interrupts(bus, device, function); // disable interrupts from every device that we do not know
}

uint8_t *get_pci_vendor_string(uint32_t vendor_id)
{
    uint32_t pci_vendor_id_string_array[256];

    for (uint32_t i = 0; i < 256; i += 2)
    {
        if (pci_vendor_id_string_array[i] == 0)
        {
            break;
        }
        else if (pci_vendor_id_string_array[i] == vendor_id)
        {
            return (uint8_t *)(pci_vendor_id_string_array[i + 1]);
        }
    }

    return ""; // this vendor id is not in list
}

void pci_probe()
{
    for (uint32_t bus = 0; bus < 256; bus++)
    {
        for (uint32_t slot = 0; slot < 32; slot++)
        {
            for (uint32_t function = 0; function < 8; function++)
            {
                uint16_t vendor = (uint16_t)(getVendorID(bus, slot, function) & 0x0000FFFF);
                if (vendor == 0xffff)
                    continue;
                /*
                uint16_t device = getDeviceID(bus, slot, function);
                printf("vendor: 0x%x device: 0x%x\n", vendor, device);
                pci_device *pdev = (pci_device *)malloc(sizeof(pci_device));
                pdev->vendor = vendor;
                pdev->device = device;
                pdev->func = function;
                pdev->driver = 0;
                add_pci_device(pdev);
                */
            }
        }
    }
}

uint16_t pciCheckVendor(uint16_t bus, uint16_t slot)
{
    uint16_t vendor = (uint16_t)(getVendorID(bus, slot, 0) & 0x0000FFFF);
    uint16_t device;
    /* check if device is valid */
    if (vendor != 0xFFFF)
    {
        device = getDeviceID(bus, slot, 0);
        /* valid device */
    }
    return (vendor);
}

void pci_init()
{
    devs = drivs = 0;
    // pci_devices = (pci_device **)malloc(32 * sizeof(pci_device));
    // pci_drivers = (pci_driver **)malloc(32 * sizeof(pci_driver));
    pci_probe();
    printf("PCI driver support loaded.\n");
}
/*
void pci_register_driver(pci_driver *driv)
{
    pci_drivers[drivs] = driv;
    drivs++;
    return;
}

void pci_proc_dump(uint8_t *buffer)
{
    for (int i = 0; i < devs; i++)
    {
        pci_device *pci_dev = pci_devices[i];
        if (pci_dev->driver)
        printf("[%x:%x:%x] => %s\n", pci_dev->vendor, pci_dev->device, pci_dev->func, pci_dev->driver->name);
        else
        printf("[%x:%x:%x]\n", pci_dev->vendor, pci_dev->device, pci_dev->func);
    }
}
*/
