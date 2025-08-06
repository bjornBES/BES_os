#include "vesa.h"
#include "x86.h"

#include "stdio.h"
#include "memdefs.h"

#include <stdbool.h>

int16_t VESAModes[] = {
    0x00,
    0x01,
    0x02,
    0x03,
    0x04,
    0x05,
    0x06,
    0x07,
    0x08,
    0x09,
    0x0A,
    0x0B,
    0x0C,
    0x0D,
    0x0E,
    0x0F,
    0x10,
    0x11,
    0x12,
    0x13,
    0x14,
    0x15,
    0x16,
    0x30,
    0x50,
    0x100,
    0x101,
    0x102,
    0x103,
    0x105,
    0x107,
    0x109,
    0x10A,
    0x110,
    0x111,
    0x112,
    0x113,
    0x114,
    0x115,
    0x116,
    0x117,
    0x122,
    0x124,
    -1,
    -1,
    -1,
    -1,
};

#define MAX_REGIONS 256

bool VESASupported;
vesa_mode_info_t g_VesaEntries[MAX_REGIONS];
VbeInfoBlock *Info;

void Detect_VESA(VESAInfo* vesaInfo)
{
    int g_VesaEntriesCount;
    Info = (VbeInfoBlock*)MEMORY_VESAINFO_ADDR;
    VESASupported = x86_VESASupported(Info);

    if (VESASupported)
    {
        uint32_t index = 0;
        int ret = 1;
        
        g_VesaEntriesCount = 0;
        
        while (ret > 0)
        {
            if (VESAModes[index] == -1)
            {
                break;
            }
            uint16_t mode = 0x4000 | (VESAModes[index] & 0x1FF);
            ret = x86_GetVESAEntry(mode, &g_VesaEntries[VESAModes[index]]);
            g_VesaEntriesCount++;
            index++;
        }
    }

    vesaInfo->SupportVESA = VESASupported;
    vesaInfo->VESACount = g_VesaEntriesCount;
    vesaInfo->entries = g_VesaEntries;
}

void SetVESAMode(int mode)
{
    // bool isGraphic = false;
    if (mode == -1)
    {
        printf("Mode is %u\n", mode);
        return;
    }
    vesa_mode_info_t vesaModeData;
    int ret = x86_GetVESAEntry((mode & 0x1FF) | 0x4000, &vesaModeData);
    if (ret == 0)
    {
        printf("ret is %u == 0\n", ret);
        return;
    }
    printf("ret = %u attributes = 0x%x\n", ret, vesaModeData.attributes);

    int vesaMode = mode | 0x4000;

    ret = x86_SetVESAMode(vesaMode);
    printf("result = %x\n", ret);
    if (ret != 0x004f)
    {
        printf("Didnt work can't set mode for vesa\n");
        // SetVGAMode(0);
    }
}