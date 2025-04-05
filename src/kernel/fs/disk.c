
#include "disk.h"
#include "drivers/ATA/ATA.h"
#include <stdint.h>

void GetDisk(IdentifyDeviceData* ATA_identify, DISK* diskout)
{
    diskout->cylinders = ATA_identify->cylinders;
    diskout->heads = ATA_identify->heads;
    diskout->sectors_per_track = ATA_identify->sectors_per_track;
}

void DISK_LBA2CHS(DISK* disk, uint32_t lba, uint16_t* cylinderOut, uint16_t* sectorOut, uint16_t* headOut)
{
    // sector = (LBA % sectors per track + 1)
    *sectorOut = lba % disk->sectors_per_track + 1;

    // cylinder = (LBA / sectors per track) / heads
    *cylinderOut = (lba / disk->sectors_per_track) / disk->heads;

    // head = (LBA / sectors per track) % heads
    *headOut = (lba / disk->sectors_per_track) % disk->heads;
}
void DISK_CHS2LBA(DISK* disk, uint16_t cylinder, uint16_t sector, uint16_t head, uint32_t* lbaOut)
{

}