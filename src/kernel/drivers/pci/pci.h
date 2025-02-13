#pragma once

#include <stdint.h>

#define PCI_BAR0 0x10
#define PCI_BAR1 0x14
#define PCI_BAR2 0x18
#define PCI_BAR3 0x1C
#define PCI_BAR4 0x20
#define PCI_BAR5 0x24

#define PCI_MMIO_BAR 0x0
#define PCI_IO_BAR 0x1

#define DEVICE_PRESENCE_IS_NOT_KNOWN 0xFF
#define DEVICE_NOT_PRESENT 0
#define DEVICE_PRESENT 1
#define DEVICE_PRESENT_BUT_ERROR_STATE 2

#define VENDOR_INTEL 0x8086
#define VENDOR_AMD_1 0x1022
#define VENDOR_AMD_2 0x1002
#define VENDOR_BROADCOM 0x14E4
#define VENDOR_REALTEK 0x10EC
#define VENDOR_QUALCOMM_ATHEROS_1 0x168C
#define VENDOR_QUALCOMM_ATHEROS_2 0x1969
#define VENDOR_NVIDIA 0x10DE
#define VENDOR_TEXAS_INSTUMENTS 0x104C
#define VENDOR_CONEXANT_SYSTEMS 0x14F1
#define VENDOR_SIGMATEL 0x8384
#define VENDOR_RED_HAT 0x1AF4

struct __pci_driver;

typedef struct {
	uint32_t vendor;
	uint32_t device;
	uint32_t func;
	struct __pci_driver *driver;
} pci_device;

typedef struct {
	uint32_t vendor;
	uint32_t device;
	uint32_t func;
} pci_device_id;

typedef struct __pci_driver {
	pci_device_id *table;
	char *name;
	uint8_t (*init_one)(pci_device *);
	uint8_t (*init_driver)(void);
	uint8_t (*exit_driver)(void);
} pci_driver;

void pci_init();

uint16_t pci_read_int16(uint32_t bus, uint32_t device, uint32_t function, uint32_t offset);
uint32_t pci_read_int32(uint32_t bus, uint32_t device, uint32_t function, uint32_t offset);
void pci_write(uint32_t bus, uint32_t device, uint32_t function, uint32_t offset, uint32_t value);
uint32_t pci_read_bar_type(uint32_t bus, uint32_t device, uint32_t function, uint32_t bar);
uint16_t pci_read_io_bar(uint32_t bus, uint32_t device, uint32_t function, uint32_t bar);
uint32_t pci_read_mmio_bar(uint32_t bus, uint32_t device, uint32_t function, uint32_t bar);
void pci_enable_io_busmastering(uint32_t bus, uint32_t device, uint32_t function);
void pci_enable_mmio_busmastering(uint32_t bus, uint32_t device, uint32_t function);
void pci_disable_interrupts(uint32_t bus, uint32_t device, uint32_t function);
void scan_pci(void);
void scan_pci_device(uint32_t bus, uint32_t device, uint32_t function);
uint8_t *get_pci_vendor_string(uint32_t vendor_id);