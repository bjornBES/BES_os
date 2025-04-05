#include "pci.h"
#include "x86.h"
#include "memory.h"
#include "stdio.h"

void Detect_PCI(PCIInfo* pciInfo)
{
    memset(pciInfo, 0, sizeof(PCIInfo));

    uint8_t     pciHWChar;
    uint8_t     pciPModeEntry;
    uint16_t    pciInterfaceLevel;
    uint8_t     pciLastBus;
    if (x86_PCIInitCheck(&pciHWChar, &pciPModeEntry, &pciInterfaceLevel, &pciLastBus) == true)
    {
        printf("PIC is installed\n");
        pciInfo->IsInstalled = true;
        pciInfo->PCIHWCharacteristics =  pciHWChar;
        pciInfo->PCIInterfaceLevel = pciInterfaceLevel;
        pciInfo->PCILastBus = pciLastBus;
        pciInfo->PCIProtectedModeEntry = pciPModeEntry;
    }
    else
    {
        printf("PIC is not installed\n");
    }
    return;
}
