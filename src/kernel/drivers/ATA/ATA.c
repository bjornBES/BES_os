#include "drivers/ATA/ATA.h"
#include "arch/i686/vga_text.h"
#include "hal/hal.h"
#include "device.h"
#include "memory.h"
#include "arch/i686/io.h"
#include "arch/i686/i8259.h"
#include "stdio.h"
#include "arch/i686/idt.h"
#include "arch/i686/gdt.h"
#include "fs/disk.h"

#include <stdint.h>

#define MODULE          "ATA"

#define ATA_PRIMARY_IO 0x1F0
#define ATA_SECONDARY_IO 0x170

#define ATA_PRIMARY_DCR_AS 0x3F6
#define ATA_SECONDARY_DCR_AS 0x376

#define ATA_PRIMARY_IRQ 14
#define ATA_SECONDARY_IRQ 15

#define ATA_MasterDrive	0xA0
#define ATA_SlaveDirve 0xB0

#define ATA_waitBusy(io) { while(i686_inw(ATA_REG_STATUS + io) & ATA_SR_BSY != 0) {} }

uint8_t ata_pm = 0; /* Primary master exists? */
uint8_t ata_ps = 0; /* Primary Slave exists? */
uint8_t ata_sm = 0; /* Secondary master exists? */
uint8_t ata_ss = 0; /* Secondary slave exists? */

uint8_t *ide_buf = 0;

bool ATA_CheakError(uint16_t io)
{
	retry2:	
    uint8_t status = i686_inb(io + ATA_REG_STATUS);
	if(status & ATA_SR_ERR)
	{
		return true;
	}
	//mprint("testing for DRQ\n");
	if(!(status & ATA_SR_DRQ)) goto retry2;
	return false;
}

bool ATA_GetIO(uint8_t bus, uint8_t* io, uint8_t* drive)
{
	uint8_t bufferIo = 0;
	uint8_t bufferDrive = 0;
	switch(bus)
	{
		case 0x0:
			bufferIo = ATA_PRIMARY_IO;
			bufferDrive = ATA_MASTER;
			break;
		case 0x1:
			bufferIo = ATA_PRIMARY_IO;
			bufferDrive = ATA_SLAVE;
			break;
		case 0x2:
			bufferIo = ATA_SECONDARY_IO;
			bufferDrive = ATA_MASTER;
			break;
		case 0x3:
			bufferIo = ATA_SECONDARY_IO;
			bufferDrive = ATA_SLAVE;
			break;
		default:
			printf("FATAL: unknown drive!\n");
			return false;
	}
	if (io != NULL)
	{
		*io = bufferIo;
	}
	if (drive != NULL)
	{
		*drive = bufferDrive;
	}
	return true;
}

void ide_select_drive(uint8_t bus, uint8_t i)
{
	if(bus == ATA_PRIMARY)
    {
		if(i == ATA_MASTER)
        {
			i686_outb(ATA_PRIMARY_IO + ATA_REG_HDDEVSEL, ATA_MasterDrive);
        }
		else 
        {
            i686_outb(ATA_PRIMARY_IO + ATA_REG_HDDEVSEL, ATA_SlaveDirve);
        }
    }
	else
    {
		if(i == ATA_MASTER)
        {
			i686_outb(ATA_SECONDARY_IO + ATA_REG_HDDEVSEL, ATA_MasterDrive);
        }
		else 
        {
            i686_outb(ATA_SECONDARY_IO + ATA_REG_HDDEVSEL, ATA_SlaveDirve);
        }
    }
}

void ide_primary_irq()
{
	i8259_SendEndOfInterrupt(ATA_PRIMARY_IRQ);
}

void ide_secondary_irq()
{
	i8259_SendEndOfInterrupt(ATA_SECONDARY_IRQ);
}

uint8_t ide_identify(uint8_t bus, uint8_t drive)
{
	uint16_t io = 0;
	ide_select_drive(bus, drive);
	ATA_GetIO(bus, &io, NULL);
	/* ATA specs say these values must be zero before sending IDENTIFY */
	i686_outb(io + ATA_REG_SECCOUNT0, 0);
	i686_outb(io + ATA_REG_LBA0, 0);
	i686_outb(io + ATA_REG_LBA1, 0);
	i686_outb(io + ATA_REG_LBA2, 0);
	/* Now, send IDENTIFY */
	i686_outb(io + ATA_REG_COMMAND, ATA_CMD_IDENTIFY);
	printf("Sent IDENTIFY\n");
	/* Now, read status port */
	uint8_t status = i686_inb(io + ATA_REG_STATUS);
	if(status)
	{
		/* Now, poll untill BSY is clear. */
		ATA_waitBusy(io);
pm_stat_read:		
        status = i686_inb(io + ATA_REG_STATUS);
		if(status & ATA_SR_ERR)
		{
			printf("%s%s has ERR set. Disabled.\n", bus==ATA_PRIMARY?"Primary":"Secondary", drive==ATA_PRIMARY?" master":" slave");
			return 0;
		}
		while(!(status & ATA_SR_DRQ)) goto pm_stat_read;
		printf("%s%s is online.\n", bus==ATA_PRIMARY?"Primary":"Secondary", drive==ATA_PRIMARY?" master":" slave");
		/* Now, actually read the data */
		//set_task(0);
		for(int i = 0; i<256; i++)
		{
			*(uint16_t *)(ide_buf + i*2) = i686_inw(io + ATA_REG_DATA);
		}
		//set_task(1);
	}
}

void ide_400ns_delay(uint16_t io)
{
	for(int i = 0;i < 4; i++)
		i686_inb(io + ATA_REG_ALTSTATUS);
}

void ide_poll(uint16_t io)
{
	
	for(int i=0; i< 4; i++)
	{
        i686_inb(io + ATA_REG_ALTSTATUS);
    }

	ATA_waitBusy(io);
	if(ATA_CheakError(io))
	{
        printf("ERR set, device failure!\n");
		panic();
	}
	return;
}

uint8_t ata_read_one(uint8_t *buf, uint32_t lba, device_t *dev)
{
	//lba &= 0x00FFFFFF; // ignore topmost byte
	/* We only support 28bit LBA so far */
	uint8_t drive = ((ide_private_data *)(dev->priv))->drive;
	uint16_t io = 0;
	ATA_GetIO(drive, &io, &drive);
	//kprintf("io=0x%x %s\n", io, drive==ATA_MASTER?"Master":"Slave");
	uint8_t cmd = (drive==ATA_MASTER?0xE0:0xF0);
	uint8_t slavebit = (drive == ATA_MASTER?0x00:0x01);
	/*kprintf("LBA = 0x%x\n", lba);
	kprintf("LBA>>24 & 0x0f = %d\n", (lba >> 24)&0x0f);
	kprintf("(uint8_t)lba = %d\n", (uint8_t)lba);
	kprintf("(uint8_t)(lba >> 8) = %d\n", (uint8_t)(lba >> 8));
	kprintf("(uint8_t)(lba >> 16) = %d\n", (uint8_t)(lba >> 16));*/
	//outportb(io + ATA_REG_HDDEVSEL, cmd | ((lba >> 24)&0x0f));
	i686_outb(io + ATA_REG_HDDEVSEL, (cmd | (uint8_t)((lba >> 24 & 0x0F))));
	//mprint("issued 0x%x to 0x%x\n", (cmd | (lba >> 24)&0x0f), io + ATA_REG_HDDEVSEL);
	//for(int k = 0; k < 10000; k++) ;
	i686_outb(io + 1, 0x00);
	//mprint("issued 0x%x to 0x%x\n", 0x00, io + 1);
	//for(int k = 0; k < 10000; k++) ;
	i686_outb(io + ATA_REG_SECCOUNT0, 1);
	//mprint("issued 0x%x to 0x%x\n", (uint8_t) numsects, io + ATA_REG_SECCOUNT0);
	//for(int k = 0; k < 10000; k++) ;
	i686_outb(io + ATA_REG_LBA0, (uint8_t)((lba)));
	//mprint("issued 0x%x to 0x%x\n", (uint8_t)((lba)), io + ATA_REG_LBA0);
	//for(int k = 0; k < 10000; k++) ;
	i686_outb(io + ATA_REG_LBA1, (uint8_t)((lba) >> 8));
	//mprint("issued 0x%x to 0x%x\n", (uint8_t)((lba) >> 8), io + ATA_REG_LBA1);
	//for(int k = 0; k < 10000; k++) ;
	i686_outb(io + ATA_REG_LBA2, (uint8_t)((lba) >> 16));
	//mprint("issued 0x%x to 0x%x\n", (uint8_t)((lba) >> 16), io + ATA_REG_LBA2);
	//for(int k = 0; k < 10000; k++) ;
	i686_outb(io + ATA_REG_COMMAND, ATA_CMD_READ_PIO);
	//mprint("issued 0x%x to 0x%x\n", ATA_CMD_READ_PIO, io + ATA_REG_COMMAND);

	ide_poll(io);

	// set_task(0);
	for(int i = 0; i < 256; i++)
	{
		uint16_t data = i686_inw(io + ATA_REG_DATA);
		*(uint16_t *)(buf + i * 2) = data;
	}
	ide_400ns_delay(io);
	// set_task(1);
	return 1;
}

void ata_read(uint8_t *buf, uint32_t lba, uint32_t numsects, device_t *dev)
{
	for(int i = 0; i < numsects; i++)
	{
		ata_read_one(buf, lba + i, dev);
		buf += 512;
	}
}

void ata_probe()
{
	/* First check the primary bus,
	 * and inside the master drive.
	 */
	
	if(ide_identify(ATA_PRIMARY, ATA_MASTER))
	{
		ata_pm = 1;
		device_t *dev = (device_t *)malloc(sizeof(device_t));
		ide_private_data *priv = (ide_private_data *)malloc(sizeof(ide_private_data));
		/* Now, process the IDENTIFY data */
		/* Model goes from W#27 to W#46 */
		char *str = (char *)malloc(40);
		for(int i = 0; i < 40; i += 2)
		{
			str[i] = ide_buf[ATA_IDENT_MODEL + i + 1];
			str[i + 1] = ide_buf[ATA_IDENT_MODEL + i];
		}
		dev->name = str;
		dev->unique_id = 32;
		dev->dev_type = DEVICE_BLOCK;
		priv->drive = (ATA_PRIMARY << 1) | ATA_MASTER;
		dev->priv = priv;
		dev->read = ata_read;
		device_add(dev);
		printf("Device: %s\n", dev->name);
	}
	ide_identify(ATA_PRIMARY, ATA_SLAVE);
	/*ide_identify(ATA_SECONDARY, ATA_MASTER);
	ide_identify(ATA_SECONDARY, ATA_SLAVE);*/
}

void ATA_init()
{
	printf("Checking for ATA drives\n");
	ide_buf = (uint16_t *)malloc(512);
    i686_IDT_SetGate(ATA_PRIMARY_IRQ, ide_primary_irq, i686_GDT_CODE_SEGMENT, IDT_FLAG_GATE_32BIT_INT | IDT_FLAG_RING0 | IDT_FLAG_PRESENT), 
    i686_IDT_SetGate(ATA_SECONDARY_IRQ, ide_secondary_irq, i686_GDT_CODE_SEGMENT, IDT_FLAG_GATE_32BIT_INT | IDT_FLAG_RING0 | IDT_FLAG_PRESENT), 
	ata_probe();
}

void ATA_identify(ATA_Identify_t* buffer)
{
	uint16_t io = 0;
	ide_select_drive(ATA_PRIMARY, ATA_MASTER);
	ATA_GetIO(ATA_PRIMARY, &io, NULL);

	i686_outb(io + ATA_REG_SECCOUNT0, 0);
	i686_outb(io + ATA_REG_LBA0, 0);
	i686_outb(io + ATA_REG_LBA1, 0);
	i686_outb(io + ATA_REG_LBA2, 0);
	i686_outb(io + ATA_REG_COMMAND, ATA_CMD_IDENTIFY);

	ATA_waitBusy(io);
	if (ATA_CheakError(io))
	{
        printf("ERR set, device failure!\n");
		panic();
	}

	uint16_t* pBuffer = (uint16_t*)buffer;
	for (int i = 0; i < 256; i++)
	{
		pBuffer[i] = i686_inw(io + ATA_REG_DATA);
	}
}
