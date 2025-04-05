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
#include "drivers/ahci/ahci.h"

#include "drivers/ATA/ATA.h"
#include "drivers/storage.h"

#include "pci.h"

#define MODULE "PCI"

Page pdevPage;

pci_device **pci_devices = 0;
uint32_t devs = 0;

uint32_t pci_devices_array_mem;
uint32_t pci_num_of_devices;

pci_driver **pci_drivers = 0;
uint32_t drivs = 0;

uint16_t headerType;

#define getAddress(bus, slot, function, offset) (uint32_t)((1U << 31) | (bus << 16) | (slot << 11) | (function << 8) | (offset));

void add_pci_device(pci_device *pdev)
{
    pci_devices[devs] = pdev;
    devs++;
    return;
}

uint32_t pciReadDword(uint32_t bus, uint32_t slot, uint32_t function, uint32_t offset)
{
    uint32_t address = getAddress(bus, slot, function, (offset & 0xFC));
    i686_outd(PCI_CONFIG_ADDRESS, address);
    return i686_ind(PCI_CONFIG_DATA);
}

void pciWriteDword(uint32_t bus, uint32_t slot, uint32_t function, uint32_t offset, uint32_t value)
{
    uint32_t address = getAddress(bus, slot, function, offset);
    i686_outd(PCI_CONFIG_ADDRESS, address);
    i686_outd(PCI_CONFIG_DATA, value);
}

uint32_t pciReadBarType(uint32_t bus, uint32_t slot, uint32_t function, uint32_t bar)
{
    uint32_t address = getAddress(bus, slot, function, bar);
    i686_outd(PCI_CONFIG_ADDRESS, address);
    return (uint16_t)(i686_ind(PCI_CONFIG_DATA) & 0x1);
}

uint16_t pciReadIOBar(uint32_t bus, uint32_t slot, uint32_t function, uint32_t bar)
{
    uint32_t address = getAddress(bus, slot, function, bar);
    i686_outd(PCI_CONFIG_ADDRESS, address);
    return (uint16_t)(i686_ind(PCI_CONFIG_DATA) & 0xFFFC);
}

uint32_t pciReadMMIOBar(uint32_t bus, uint32_t slot, uint32_t function, uint32_t bar)
{
    uint32_t address = getAddress(bus, slot, function, bar);
    i686_outd(PCI_CONFIG_ADDRESS, address);
    return (i686_ind(PCI_CONFIG_DATA) & 0xFFFFFFF0);
}

void pciEnableIOBusmastering(uint32_t bus, uint32_t device, uint32_t function)
{
    pciWriteDword(bus, device, function, 0x04, ((pciReadDword(bus, device, function, 0x04) & ~(1 << 10)) | (1 << 2) | (1 << 0))); // enable interrupts, enable bus mastering, enable IO space
}

void pciEnableMMIOBusmastering(uint32_t bus, uint32_t device, uint32_t function)
{
    pciWriteDword(bus, device, function, 0x04, ((pciReadDword(bus, device, function, 0x04) & ~(1 << 10)) | (1 << 2) | (1 << 1))); // enable interrupts, enable bus mastering, enable MMIO space
}

void pciDisableInterrupts(uint32_t bus, uint32_t device, uint32_t function)
{
    pciWriteDword(bus, device, function, 0x04, (pciReadDword(bus, device, function, 0x04) | (1 << 10))); // disable interrupts
}

uint32_t getStorageBAR(uint32_t bus, uint32_t device, uint32_t function, uint8_t barIndex)
{
    if (pciReadBarType(bus, device, function, 0x10 + (barIndex)) == 1)
    {
        return pciReadIOBar(bus, device, function, 0x10 + (barIndex));
    }
    else
    {
        return pciReadMMIOBar(bus, device, function, 0x10 + (barIndex));
    }
}

uint16_t getVendorID(uint32_t bus, uint32_t device, uint32_t function)
{
    uint16_t r0 = pciReadDword(bus, device, function, 0);
    return r0;
}
uint16_t getDeviceID(uint32_t bus, uint32_t device, uint32_t function)
{
    uint16_t r0 = pciReadDword(bus, device, function, 2);
    return r0;
}
uint16_t getClassId(uint16_t bus, uint16_t device, uint16_t function)
{
    uint16_t r0 = pciReadDword(bus, device, function, 0xA);
    return (r0 & ~0x00FF) >> 8;
}
uint16_t getSubClassId(uint16_t bus, uint16_t device, uint16_t function)
{
    uint16_t r0 = pciReadDword(bus, device, function, 0xA);
    return (r0 & ~0xFF00);
}

void pciScan(void)
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
    pci_num_of_devices = 0;

    log_debug(MODULE, "PCI devices:");

    for (int bus = 0; bus < 256; bus++)
    {
        for (int device = 0; device < 32; device++)
        {
            uint16_t vendor = getVendorID(bus, device, 0);
            if (vendor == 0xFFFF)
            {
                continue;
            }
            pciScanDevice(bus, device, 0);

            // multifunctional device
            if ((pciReadDword(bus, device, 0, 0x0C) & 0x00800000) == 0x00800000)
            {
                for (int function = 1; function < 8; function++)
                {
                    uint16_t vendor = getVendorID(bus, device, function);
                    if (vendor == 0xFFFF)
                    {
                        continue;
                    }
                    pciScanDevice(bus, device, function);
                }
            }
        }
    }
}

void pciScanDevice(uint32_t bus, uint32_t device, uint32_t function)
{
    uint32_t mmioPortBase;
    uint16_t ioPortBase;

    // read base informations about device
    uint16_t vendorID = (getVendorID(bus, device, function) & 0xFFFF);
    uint16_t deviceID = (getVendorID(bus, device, function) >> 16);

    uint32_t fullDeviceId = pciReadDword(bus, device, function, 0);
    if (fullDeviceId == 0xFFFFFFFF)
    {
        return; // no device
    }
    uint32_t typeOfDevice = (pciReadDword(bus, device, function, 0x08) >> 8);
    uint32_t classID = (typeOfDevice >> 16);
    uint32_t subclassID = ((typeOfDevice >> 8) & 0xFF);
    uint32_t progif = (typeOfDevice & 0xFF);
    uint32_t deviceIrq = (pciReadDword(bus, device, function, 0x3C) & 0xFF);

    log_debug(MODULE, "PCI Device Found: Bus %d, Device %d, Function %d, Vendor: 0x%X, Device: 0x%X, Class: 0x%X, Subclass: 0x%X",
              bus, device, function, vendorID, deviceID, classID, subclassID);

    if (classID == 1)
    {
        // IDE
        if (subclassID == 0x1 && number_of_storage_controllers < MAX_NUMBER_OF_STORAGE_CONTROLLERS)
        {
            pci_device_ide_controller:
            log_debug(MODULE, "IDE controller");
            pciEnableIOBusmastering(bus, device, function);
            pciDisableInterrupts(bus, device, function);
        }

        // FDD
        if (subclassID == 0x2 && number_of_storage_controllers < MAX_NUMBER_OF_STORAGE_CONTROLLERS)
        {
            log_debug(MODULE, "FDD controller");
            pciEnableIOBusmastering(bus, device, function);
            pciDisableInterrupts(bus, device, function);
        }

        // SATA controller
        if (subclassID == 0x06 && number_of_storage_controllers < MAX_NUMBER_OF_STORAGE_CONTROLLERS)
        {
            if (progif == 0x01)
            {
                log_debug(MODULE, "AHCI controller");
                pciEnableMMIOBusmastering(bus, device, function);
                pciDisableInterrupts(bus, device, function);

                uint32_t mmioBase = pciReadMMIOBar(bus, device, function, PCI_BAR5);
                log_debug(MODULE, "AHCI MMIO Base: 0x%x", mmioBase);
                AHCI_init(mmioBase);
            
                number_of_storage_controllers++;
            }
        }
    }

    // Network Controller
    else if (classID == 0x02)
    {
        // Ethernet
        if (subclassID == 0x0)
        {
            log_debug(MODULE, "Ethernet Controller");
        }
    }

    // Display Controller
    else if (classID == 0x03)
    {
        // VGA
        if (subclassID == 0x0)
        {
            log_debug(MODULE, "VGA controller");
        }
    }

    // Multimedia Controller
    else if (classID == 0x04)
    {
        // Audio Device
        if (subclassID == 0x3)
        {
            log_debug(MODULE, "Audio Device");
        }
    }

    // Bridge
    else if (classID == 0x06)
    {
        // Host
        if (subclassID == 0x0)
        {
            log_debug(MODULE, "Host Bridge");
        }

        // ISA
        if (subclassID == 0x1)
        {
            log_debug(MODULE, "ISA Bridge");
        }

        // Other
        if (subclassID == 0x80)
        {
            log_debug(MODULE, "Other");
        }
    }

    /*
    else if (classID == 0x02)
    {
        log_debug(MODULE, "Found Network Controller at Bus %d, Device %d", bus, device);
        log_debug(MODULE, "Subclass ID: 0x%X", subclassID);
    }
    else if (classID == 0x03)
    {
        log_debug(MODULE, "Found Display Controller at Bus %d, Device %d", bus, device);
        log_debug(MODULE, "Subclass ID: 0x%X", subclassID);
    }
    else if (classID == 0x04)
    {
        log_debug(MODULE, "Found Multimedia Controller at Bus %d, Device %d", bus, device);
        log_debug(MODULE, "Subclass ID: 0x%X", subclassID);
    }
    else if (classID == 0x05)
    {
        log_debug(MODULE, "Found Memory Controller at Bus %d, Device %d", bus, device);
        log_debug(MODULE, "Subclass ID: 0x%X", subclassID);
    }
    else if (classID == 0x06)
    {
        log_debug(MODULE, "Found Bridge at Bus %d, Device %d", bus, device);
        log_debug(MODULE, "Subclass ID: 0x%X", subclassID);
    }
    else if (classID == 0x07)
    {
        log_debug(MODULE, "Found Simple Communication Controller at Bus %d, Device %d", bus, device);
        log_debug(MODULE, "Subclass ID: 0x%X", subclassID);
    }
    else if (classID == 0x08)
    {
        log_debug(MODULE, "Found Base System Peripheral at Bus %d, Device %d", bus, device);
        log_debug(MODULE, "Subclass ID: 0x%X", subclassID);
    }
    else if (classID == 0x09)
    {
        log_debug(MODULE, "Found Input Device Controller at Bus %d, Device %d", bus, device);
        log_debug(MODULE, "Subclass ID: 0x%X", subclassID);
    }
    else if (classID == 0x0A)
    {
        log_debug(MODULE, "Found Docking Station at Bus %d, Device %d", bus, device);
        log_debug(MODULE, "Subclass ID: 0x%X", subclassID);
    }
    else if (classID == 0x0B)
    {
        log_debug(MODULE, "Found Processor at Bus %d, Device %d", bus, device);
        log_debug(MODULE, "Subclass ID: 0x%X", subclassID);
    }
    else if (classID == 0x0C)
    {
        log_debug(MODULE, "Found Serial Bus Controller at Bus %d, Device %d", bus, device);
        log_debug(MODULE, "Subclass ID: 0x%X", subclassID);
    }
    */

    /*

    // save device to array
    if (pci_num_of_devices < 1000)
    {
        uint32_t *pci_devices_array = (uint32_t *)(pci_devices_array_mem + pci_num_of_devices * 12);
        pci_devices_array[0] = (bus | (device << 8) | (function << 16));
        pci_devices_array[1] = typeOfDevice;
        pci_devices_array[2] = fullDeviceId;
        pci_num_of_devices++;
    }

    // Graphic card
    if(typeOfDevice==0x030000 && number_of_graphic_cards<MAX_NUMBER_OF_GRAPHIC_CARDS) {
     log_debug(MODULE, "\nGraphic card");

     if(number_of_graphic_cards>=MAX_NUMBER_OF_GRAPHIC_CARDS) {
      return;
     }

     graphic_cards_info[number_of_graphic_cards].vendor_id = vendor_id;
     graphic_cards_info[number_of_graphic_cards].device_id = device_id;

     if(vendor_id==VENDOR_INTEL) {
      pciEnableIOBusmastering(bus, device, function);
      graphic_cards_info[number_of_graphic_cards].mmio_base = pciReadMMIOBar(bus, device, function, PCI_BAR0);
      graphic_cards_info[number_of_graphic_cards].linear_frame_buffer = (uint8_t *) (pciReadMMIOBar(bus, device, function, PCI_BAR2));
     }

     number_of_graphic_cards++;
     return;
    }

    // IDE controller
    if ((typeOfDevice & 0xFFFF00) == 0x010100 && number_of_storage_controllers < MAX_NUMBER_OF_STORAGE_CONTROLLERS)
    {
    pci_device_ide_controller:
        log_debug(MODULE, "\nIDE controller");
        pciEnableIOBusmastering(bus, device, function);
        pciDisableInterrupts(bus, device, function);

        // first controller
        storage_controllers[number_of_storage_controllers].controller_type = IDE_CONTROLLER;
        ioPortBase = pciReadIOBar(bus, device, function, PCI_BAR0);
        if (ioPortBase == 0 || ide_is_bus_floating(ioPortBase) == STATUS_FALSE)
        {
            if (ioPortBase == 0 || ioPortBase == 0x1F0)
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
                storage_controllers[number_of_storage_controllers].base_1 = ioPortBase;
                storage_controllers[number_of_storage_controllers].base_2 = pciReadIOBar(bus, device, function, PCI_BAR1);
                number_of_storage_controllers++;
            }
        }
        if (number_of_storage_controllers >= MAX_NUMBER_OF_STORAGE_CONTROLLERS)
        {
            return;
        }

        // second controller
        storage_controllers[number_of_storage_controllers].controller_type = IDE_CONTROLLER;
        ioPortBase = pciReadIOBar(bus, device, function, PCI_BAR2);
        if (ioPortBase == 0 || ide_is_bus_floating(ioPortBase) == STATUS_FALSE)
        {
            if (ioPortBase == 0 || ioPortBase == 0x170)
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
                storage_controllers[number_of_storage_controllers].base_1 = ioPortBase;
                storage_controllers[number_of_storage_controllers].base_2 = pciReadIOBar(bus, device, function, PCI_BAR3);
                number_of_storage_controllers++;
            }
        }

        return;
    }

    // SATA controller
    if (typeOfDevice == 0x010601 && number_of_storage_controllers < MAX_NUMBER_OF_STORAGE_CONTROLLERS)
    {
        log_debug(MODULE, "\nAHCI controller");
        pciEnableMMIOBusmastering(bus, device, function);
        pciDisableInterrupts(bus, device, function);

        // test presence of AHCI interface
        if ((i686_ind(pciReadMMIOBar(bus, device, function, PCI_BAR5) + 0x04) & 0x80000000) == 0x00000000)
        {
            log_debug(MODULE, " with IDE interface");
            goto pci_device_ide_controller; // IDE interface
        }

        // save device
        storage_controllers[number_of_storage_controllers].controller_type = AHCI_CONTROLLER;
        storage_controllers[number_of_storage_controllers].base_1 = pciReadMMIOBar(bus, device, function, PCI_BAR5);
        number_of_storage_controllers++;
        return;
    }

    // Ethernet card
    if (typeOfDevice == 0x020000)
    {
        log_debug(MODULE, "\nEthernet card ");
        log_hex_with_space(fullDeviceId);

        if (number_of_ethernet_cards >= MAX_NUMBER_OF_ETHERNET_CARDS)
        {
            return;
        }

        // read basic values
        ethernet_cards[number_of_ethernet_cards].id = fullDeviceId;
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

            ethernet_cards[number_of_ethernet_cards].bar_type = pciReadBarType(bus, device, function, PCI_BAR0);
            if (ethernet_cards[number_of_ethernet_cards].bar_type == PCI_MMIO_BAR)
            {
                ethernet_cards[number_of_ethernet_cards].base = pciReadMMIOBar(bus, device, function, PCI_BAR0);
                pciEnableMMIOBusmastering(bus, device, function);
            }
            else
            {
                ethernet_cards[number_of_ethernet_cards].base = pciReadIOBar(bus, device, function, PCI_BAR0);
                pciEnableIOBusmastering(bus, device, function);
            }
        }

        // AMD PC-net
        else if (vendor_id == VENDOR_AMD_1 || vendor_id == VENDOR_AMD_2)
        {
            // connect to driver for Amd PCNET
            ethernet_cards[number_of_ethernet_cards].initalize = ec_amd_pcnet_initalize;

            pciEnableIOBusmastering(bus, device, function);
            ethernet_cards[number_of_ethernet_cards].bar_type = PCI_IO_BAR;
            ethernet_cards[number_of_ethernet_cards].base = pciReadIOBar(bus, device, function, PCI_BAR0);
        }

        // Broadcom NetXtreme
        else if (vendor_id == VENDOR_BROADCOM)
        {
            // connect to driver for Broadcom NetXtreme
            ethernet_cards[number_of_ethernet_cards].initalize = ec_broadcom_netxtreme_initalize;

            pciEnableMMIOBusmastering(bus, device, function);
            ethernet_cards[number_of_ethernet_cards].bar_type = PCI_MMIO_BAR;
            ethernet_cards[number_of_ethernet_cards].base = pciReadMMIOBar(bus, device, function, PCI_BAR0);
        }

        // Qualcomm Atheros
        else if (vendor_id == VENDOR_QUALCOMM_ATHEROS_1 || vendor_id == VENDOR_QUALCOMM_ATHEROS_2)
        {
            // connect to driver for Qualcomm Atheros
            ethernet_cards[number_of_ethernet_cards].initalize = ec_atheros_initalize;

            pciEnableIOBusmastering(bus, device, function);
            pciEnableMMIOBusmastering(bus, device, function);
            ethernet_cards[number_of_ethernet_cards].bar_type = PCI_MMIO_BAR;
            ethernet_cards[number_of_ethernet_cards].base = pciReadMMIOBar(bus, device, function, PCI_BAR0);
        }

        // Realtek
        else if (vendor_id == VENDOR_REALTEK)
        {
            if (device_id == 0x8139)
            {
                // connect to driver for Realtek 8139
                ethernet_cards[number_of_ethernet_cards].initalize = ec_realtek_8139_initalize;

                pciEnableIOBusmastering(bus, device, function);
                ethernet_cards[number_of_ethernet_cards].bar_type = PCI_IO_BAR;
                ethernet_cards[number_of_ethernet_cards].base = pciReadIOBar(bus, device, function, PCI_BAR0);
            }
            else if (device_id == 0x8136 || device_id == 0x8161 || device_id == 0x8168 || device_id == 0x8169)
            {
                // connect to driver for Realtek 8169
                ethernet_cards[number_of_ethernet_cards].initalize = ec_realtek_8169_initalize;

                ethernet_cards[number_of_ethernet_cards].bar_type = pciReadBarType(bus, device, function, PCI_BAR0);
                if (ethernet_cards[number_of_ethernet_cards].bar_type == PCI_MMIO_BAR)
                {
                    ethernet_cards[number_of_ethernet_cards].base = pciReadMMIOBar(bus, device, function, PCI_BAR0);
                    pciEnableMMIOBusmastering(bus, device, function);
                }
                else
                {
                    ethernet_cards[number_of_ethernet_cards].base = pciReadIOBar(bus, device, function, PCI_BAR0);
                    pciEnableIOBusmastering(bus, device, function);
                }
            }
        }

        number_of_ethernet_cards++;
        return;
    }

    // Ethernet card
    if (typeOfDevice == 0x028000)
    {
        log_debug(MODULE, "\nWireless card ");
        log_hex_with_space(fullDeviceId);

        return;
    }

    // AC97 sound card
    if (typeOfDevice == 0x040100 && number_of_sound_cards < MAX_NUMBER_OF_SOUND_CARDS)
    {
        log_debug(MODULE, "\nsound card AC97");

        if (number_of_sound_cards >= MAX_NUMBER_OF_SOUND_CARDS)
        {
            return;
        }

        pciEnableIOBusmastering(bus, device, function);

        sound_cards_info[number_of_sound_cards].driver = SOUND_CARD_DRIVER_AC97;
        sound_cards_info[number_of_sound_cards].vendor_id = vendor_id;
        sound_cards_info[number_of_sound_cards].device_id = device_id;
        sound_cards_info[number_of_sound_cards].io_base = pciReadIOBar(bus, device, function, PCI_BAR0);
        sound_cards_info[number_of_sound_cards].io_base_2 = pciReadIOBar(bus, device, function, PCI_BAR1);
        number_of_sound_cards++;

        return;
    }

    // HD Audio sound card
    if (typeOfDevice == 0x040300 && number_of_sound_cards < MAX_NUMBER_OF_SOUND_CARDS)
    {
        log_debug(MODULE, "\nsound card HDA");

        if (number_of_sound_cards >= MAX_NUMBER_OF_SOUND_CARDS)
        {
            return;
        }

        pciEnableMMIOBusmastering(bus, device, function);

        sound_cards_info[number_of_sound_cards].driver = SOUND_CARD_DRIVER_HDA;
        sound_cards_info[number_of_sound_cards].vendor_id = vendor_id;
        sound_cards_info[number_of_sound_cards].device_id = device_id;
        sound_cards_info[number_of_sound_cards].mmio_base = pciReadMMIOBar(bus, device, function, PCI_BAR0);
        number_of_sound_cards++;

        return;
    }

    // Universal Host Controller Interface
    if (typeOfDevice == 0x0C0300)
    {
        log_debug(MODULE, "\nUHCI");

        if (usb_controllers_pointer >= MAX_NUMBER_OF_USB_CONTROLLERS)
        {
            return;
        }

        usb_controllers[usb_controllers_pointer].type = USB_CONTROLLER_UHCI;
        usb_controllers[usb_controllers_pointer].base = pciReadIOBar(bus, device, function, PCI_BAR4);
        usb_controllers_pointer++;

        // disable BIOS legacy support
        pciWriteDword(bus, device, function, 0xC0, 0x8F00);

        return;
    }

    // Open Host Controller Interface
    if (typeOfDevice == 0x0C0310)
    {
        log_debug(MODULE, "\nOHCI");

        if (usb_controllers_pointer >= MAX_NUMBER_OF_USB_CONTROLLERS)
        {
            return;
        }

        usb_controllers[usb_controllers_pointer].type = USB_CONTROLLER_OHCI;
        usb_controllers[usb_controllers_pointer].base = pciReadMMIOBar(bus, device, function, PCI_BAR0);
        usb_controllers_pointer++;

        // disable BIOS legacy support
        if (mmio_ind(usb_controllers[usb_controllers_pointer].base + 0x0) == 0x110)
        {
            mmio_outd(usb_controllers[usb_controllers_pointer].base + 0x100, 0x00);
        }

        return;
    }

    // Enhanced Host Controller Interface
    if (typeOfDevice == 0x0C0320)
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
        usb_controllers[usb_controllers_pointer].base = pciReadMMIOBar(bus, device, function, PCI_BAR0);
        usb_controllers_pointer++;

        return;
    }

    // eXtensible Host Controller Interface
    if (typeOfDevice == 0x0C0330)
    {
        log_debug(MODULE, "\nxHCI");

        return;
    }

    // Host bridge
    if (typeOfDevice == 0x060000)
    {
        log_debug(MODULE, "\nHost bridge");

        return;
    }

    // ISA bridge
    if (typeOfDevice == 0x060100)
    {
        log_debug(MODULE, "\nISA bridge");

        return;
    }

    // PCI bridge
    if ((typeOfDevice & 0xFFFF00) == 0x060400)
    {
        log_debug(MODULE, "\nPCI bridge");

        return;
    }

    // Other bridge
    if (typeOfDevice == 0x068000)
    {
        log_debug(MODULE, "\nOther bridge");

        return;
    }

    log_debug(MODULE, "\nunknown device %X", typeOfDevice);
    pciDisableInterrupts(bus, device, function); // disable interrupts from every device that we do not know
    */
}

void pci_probe()
{
    for (uint32_t bus = 0; bus < 256; bus++)
    {
        for (uint32_t slot = 0; slot < 32; slot++)
        {
            uint16_t vendor = (uint16_t)(getVendorID(bus, slot, 0));
            if (vendor == 0xffff)
                continue;
            uint16_t device = getDeviceID(bus, slot, 0);
            printf("bus: 0x%x slot: 0x%x vendor: 0x%x device: 0x%x\n", bus, slot, vendor, device);
            pci_device *pdev = (pci_device *)malloc(sizeof(pci_device), &pdevPage);
            pdev->vendor = vendor;
            pdev->device = device;
            pdev->func = 0;
            pdev->driver = 0;
            add_pci_device(pdev);
        }
    }
}

void pci_init(uint8_t HWChar)
{
    devs = drivs = 0;
    // pci_devices = (pci_device **)malloc(32 * sizeof(pci_device));
    // pci_drivers = (pci_driver **)malloc(32 * sizeof(pci_driver));
    pci_probe();
    printf("PCI driver support loaded.\n");
}
