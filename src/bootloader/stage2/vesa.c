#include "vesa.h"
#include "x86.h"

#include "stdio.h"

#include <stdbool.h>

uint16_t VESAModes[] = {
    0x03,
    0x10D,
    0x10E,
    0x10F,
    0x101,
    0x103,
    0x105,
    0x107,
    0x110,
    0x111,
    0x112,
    0x114,
    0x115,
    0x116,
    0x117,
    0x118,
    0x119,
    0x11A,
    0x11B,
    0x11C,
    0,
};

#define MAX_REGIONS 256

bool VESASupported;
vesa_mode_info_t g_VesaEntries[MAX_REGIONS];
int g_VesaEntriesCount;

void Detect_VESA(VESAInfo* vesaInfo)
{
    VbeInfoBlock Info;
    VESASupported = x86_VESASupported(&Info);

    if (VESASupported)
    {
        uint32_t index = 0;
        int ret = 1;
        
        g_VesaEntriesCount = 0;
        
        while (ret > 0 && VESAModes[index] != 0)
        {
            ret = x86_GetVESAEntry(VESAModes[index], &g_VesaEntries[g_VesaEntriesCount]);
            g_VesaEntriesCount++;
            index++;
        }
    }

    vesaInfo->SupportVESA = VESASupported;
    vesaInfo->VESACount = g_VesaEntriesCount;
    vesaInfo->entries = g_VesaEntries;
}