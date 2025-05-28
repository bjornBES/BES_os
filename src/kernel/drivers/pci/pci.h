#pragma once

#include <stdint.h>

#define PCI_CONFIG_ADDRESS  0xCF8
#define PCI_CONFIG_DATA     0xCFC

// PCI config header
#define PCI_VENDOR_ID 0x00
#define PCI_DEVICE_ID 0x02
#define PCI_COMMAND 0x04
#define PCI_STATUS 0x06
#define PCI_REVISION_ID 0x08
#define PCI_PROG_IF 0x09
#define PCI_SUBCLASS 0x0a
#define PCI_CLASS 0x0b
#define PCI_CACHE_LINE_SIZE 0x0c
#define PCI_LATENCY_TIMER 0x0d
#define PCI_HEADER_TYPE 0x0e
#define PCI_BIST 0x0f
#define PCI_BAR0 0x10
#define PCI_BAR1 0x14
#define PCI_BAR2 0x18
#define PCI_BAR3 0x1C
#define PCI_BAR4 0x20
#define PCI_BAR5 0x24
#define PCI_SECONDARY_BUS 0x09
#define PCI_SYSTEM_VENDOR_ID 0x2C
#define PCI_SYSTEM_ID 0x2E
#define PCI_EXP_ROM_BASE_ADDR 0x30
#define PCI_CAPABILITIES_PTR 0x34
#define PCI_INTERRUPT_LINE 0x3C
#define PCI_MIN_GRANT 0x3E

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
	uint32_t BAR0;
	uint32_t BAR1;
	uint32_t BAR2;
	uint32_t BAR3;
	uint32_t BAR4;
	uint32_t BAR5;
	uint32_t CardbusCIS;
	uint16_t SubsystemID;
	uint16_t SubsystemVendorID;
	uint32_t ROMBaseAddress;
	uint8_t Reserved[3];
	uint8_t CapabilitiesPointer;
	uint32_t Reserved2;
	uint8_t MaxLatency;
	uint8_t MinGnt;
	uint8_t InterruptPin;
	uint8_t InterruptLine;
} PCIHeader0;

typedef struct {
	uint32_t BAR1;
	uint32_t BAR2;

	uint8_t SecondaryLatencyTimer;
	uint8_t SubordinateBusNumber;
	uint8_t SecondaryBusNumber;
	uint8_t PrimaryBusNumber;
	uint16_t SecondaryStatus;
	uint8_t IOLimit;
	uint8_t IOBase;
	uint16_t MemoryLimit;
	uint16_t MemoryBase;
	uint16_t PrefetchableMemoryLimit;
	uint16_t PrefetchableMemoryBase;
	uint32_t PrefetchableBaseUpper32;
	uint32_t PrefetchableLimitUpper32;
	uint16_t IOLimitUpper16;
	uint16_t IOBaseUpper16;
	uint8_t Reserved[3];
	uint8_t CapabilitiesPtr;
	uint32_t ROMBaseAddress;
	uint16_t BridgeControlRegister;
	uint8_t InterruptPin;
	uint8_t InterruptLine;
} PCIHeader1;

typedef union {
	PCIHeader0 header0;
	PCIHeader1 header1;
	uint8_t bytes[48];
} PCIHeader;


typedef struct {
	uint16_t bus;
	uint16_t slot;
	uint16_t function;

	uint16_t vendorID;
	uint16_t deviceID;

	uint16_t command;
	uint16_t status;

	uint8_t revision;
	uint8_t progif;
	uint8_t subclassID;
	uint8_t classID;

	uint8_t cacheLineSize;
	uint8_t latencyTimer;
	uint8_t headerType;
	uint8_t BIST;

	PCIHeader header;
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

uint32_t pciReadDword(uint32_t bus, uint32_t device, uint32_t function, uint32_t offset);
void pciWriteDword(uint32_t bus, uint32_t device, uint32_t function, uint32_t offset, uint32_t value);
void pci_init(uint8_t HWChar);

uint32_t pciReadBarType(uint32_t bus, uint32_t device, uint32_t function, uint32_t bar);
uint16_t pciReadIOBar(uint32_t bus, uint32_t device, uint32_t function, uint32_t bar);
uint32_t pciReadMMIOBar(uint32_t bus, uint32_t device, uint32_t function, uint32_t bar);

void pciEnableIOBusmastering(uint32_t bus, uint32_t device, uint32_t function);
void pciEnableMMIOBusmastering(uint32_t bus, uint32_t device, uint32_t function);
void pciDisableInterrupts(uint32_t bus, uint32_t device, uint32_t function);

uint32_t getStorageBAR(uint32_t bus, uint32_t device, uint32_t function, uint8_t barIndex);

void pciScan(void);
void pciScanDevice(pci_device *pciDevice, uint32_t bus, uint32_t device, uint32_t function);
uint8_t *get_pci_vendor_string(uint32_t vendor_id);