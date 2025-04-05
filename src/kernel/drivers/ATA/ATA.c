#include "ATA.h"

#include "arch/i686/vga_text.h"
#include "arch/i686/io.h"
#include "arch/i686/i8259.h"
#include "arch/i686/idt.h"
#include "arch/i686/gdt.h"
#include "arch/i686/isr.h"
#include "arch/i686/irq.h"

#include "memory.h"
#include "stdio.h"
#include "debug.h"
#include "malloc.h"

#include "drivers/ide/ide_controller.h"

#include "fs/disk.h"

#include "hal/hal.h"
#include "hal/vfs.h"

#include "common.h"

#include <stdint.h>

#define MODULE "ATA"

#define ATA_PRIMARY_IO (uint16_t)0x1F0
#define ATA_SECONDARY_IO (uint16_t)0x170

#define ATATOCHANNEL(channel) ((channel == ATA_PRIMARY_IO) ? ATA_PRIMARY : ATA_SECONDARY)

#define ATA_PRIMARY_DCR_AS 0x3F6
#define ATA_SECONDARY_DCR_AS 0x376

#define ATA_PRIMARY_IRQ 46
#define ATA_SECONDARY_IRQ 47

#define ATA_MasterDrive 0xA0
#define ATA_SlaveDirve 0xB0

Page *ATAPageIdentify;
Page *ATAPage;

uint8_t ATA_CheakStatus(uint8_t flag)
{
	while (true)
	{
		uint8_t f = i686_inb(ATA_REG_STATUS + ATA_PRIMARY_IO);
		if (f & flag)
		{
			break;
		}
	}
	return 1;
}
#define ATA_waitBusy()                                             \
	{                                                              \
		while (true)                                               \
		{                                                          \
			uint8_t f = i686_inb(ATA_REG_STATUS + ATA_PRIMARY_IO); \
			if ((f & ATA_SR_BSY) == 0)                             \
			{                                                      \
				break;                                             \
			}                                                      \
		}                                                          \
	}

uint16_t ata_Device;

bool ATA_CheakError(uint16_t io)
{
	while (true)
	{
		uint8_t status = i686_inb(io + ATA_REG_STATUS);
		// log_debug(MODULE, "in CheakError line %u status = %u IO = %X", __LINE__, status, io + ATA_REG_STATUS);
		if ((status & (ATA_SR_ERR | ATA_SR_DF)))
		{
			printf("Error here on line %u\n", __LINE__);
			printf("status = %u\n", status);
			printf("io = %X\n", io);
			printf("error bit mask = %u\n", status & ATA_SR_ERR);
			return true;
		}
		else
		{
			break;
		}
	}
	return false;
}

bool ATA_GetIO(uint8_t device, uint16_t *io, uint16_t *drive)
{
	IDEChannelRegisters channel = channels[ide_devices[device].Channel];
	uint16_t bufferIoBase = channel.base;
	uint16_t bufferDrive = ide_devices[device].Drive;

	if (io != NULL)
	{
		*io = bufferIoBase;
	}
	if (drive != NULL)
	{
		*drive = bufferDrive;
	}
	return true;
}

void ide_select_drive(uint8_t channel, uint8_t drive)
{
	if (channel == ATA_PRIMARY)
	{
		if (drive == ATA_MASTER)
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
		if (drive == ATA_MASTER)
		{
			i686_outb(ATA_SECONDARY_IO + ATA_REG_HDDEVSEL, ATA_MasterDrive);
		}
		else
		{
			i686_outb(ATA_SECONDARY_IO + ATA_REG_HDDEVSEL, ATA_SlaveDirve);
		}
	}
}

void ide_primary_irq(Registers *regs)
{
	log_debug(MODULE, "ATA PRIMARY IRQ HIT");
	uint8_t status = i686_inb(ATA_PRIMARY_IO + ATA_REG_STATUS);
	if ((status & ATA_SR_ERR) == ATA_SR_ERR)
	{
		ide_print_error(ATA_PRIMARY, 2);
	}

	i8259_SendEndOfInterrupt(ATA_PRIMARY_IRQ);
}

void ide_secondary_irq(Registers *regs)
{
	log_debug(MODULE, "ATA SECONDARY IRQ HIT");
	uint8_t status = i686_inb(ATA_SECONDARY_IO + ATA_REG_STATUS);
	if ((status & ATA_SR_ERR) == ATA_SR_ERR)
	{
		ide_print_error(ATA_SECONDARY, 2);
	}

	i8259_SendEndOfInterrupt(ATA_SECONDARY_IRQ);
}

void ide_poll(uint16_t ioBase)
{
	ATA_waitBusy();
	if (ATA_CheakError(ioBase))
	{
		printf("ERR set, device failure!\n");
		panic();
	}
	return;
}

void ATACacheFlush(uint16_t channel)
{
	ide_write(channel, ATA_REG_COMMAND, ATA_CMD_CACHE_FLUSH);
	ATA_waitBusy();
	ATA_CheakStatus(ATA_SR_DRQ);
}

void ata_read_one(uint8_t *buf, uint32_t lba, uint32_t offset, device_t *device)
{
	lba &= 0x00FFFFFF; /* 24 bit LBA */
	/* We only support LBA so far */
	uint16_t channel = 0;
	uint16_t drive = 0;

	uint16_t ATADevice = ((ide_private_data *)(device->priv))->drive;
	ATA_GetIO(ATADevice, &channel, &drive);

	uint8_t cmd = (drive == ATA_MASTER ? 0xE0 : 0xF0);

	log_err(MODULE, "drive = %X", drive);

	log_err(MODULE, "channel = %X, drive = %X", channel, drive);

	i686_outb(channel + ATA_REG_HDDEVSEL, (cmd | (uint8_t)((lba >> 24 & 0x0F))));
	i686_outb(channel + 1, 0x00);
	i686_outb(channel + ATA_REG_SECCOUNT0, 1);
	i686_outb(channel + ATA_REG_LBA0, (uint8_t)((lba)));
	i686_outb(channel + ATA_REG_LBA1, (uint8_t)((lba) >> 8));
	i686_outb(channel + ATA_REG_LBA2, (uint8_t)((lba) >> 16));
	i686_outb(channel + ATA_REG_COMMAND, ATA_CMD_READ_PIO);
	log_warn(MODULE, "after sending command");

	ATA_CheakStatus(ATA_SR_DRQ);

	ide_poll(channel);
	log_warn(MODULE, "after polling");

	uint16_t count = 0;
	// set_task(0);
	// ide_read_buffer(channel, ATA_REG_DATA, (uint32_t)buf, 256);
	for (int i = 0; i < 256; i++)
	{
		uint16_t data = i686_inw(channel + ATA_REG_DATA);
		uint8_t b1 = data & 0x00FF;
		uint8_t b2 = (data >> 8);

		buf[(i * 2) + offset] = b1;
		buf[(i * 2) + offset + 1] = b2;
		// log_debug(MODULE, "%X, %X", b1, b2);
		count++;
	}
	log_warn(MODULE, "readed %u words", count);

	ide_400ns_delay(channel);

	// ATACacheFlush(channel);
	// set_task(1);
}

bool ATA_read(uint8_t *buf, uint32_t lba, uint32_t numsects, device_t *device)
{
	uint32_t offset = 0;
	log_info(MODULE, "Reading %u sectors from LBA %u", numsects, lba);
	for (int i = 0; i < numsects; i++)
	{
		log_debug(MODULE, "Reading LBA %u, offset %u", lba + i, offset);
		ata_read_one(buf, lba + i, offset, device);
		offset += 512;
	}
	return true;
}

void ATA_init()
{
	printf("Checking for ATA drives\n");
	if (ide_initialize(0x1F0, 0x3F6, 0x170, 0x376, 0x000) == false)
	{
		return;
	}

	i686_IRQ_RegisterHandler(ATA_PRIMARY_IRQ, ide_primary_irq);
	i686_IRQ_RegisterHandler(ATA_SECONDARY_IRQ, ide_secondary_irq);

	// log_info(MODULE, "IdentifyPage = 0x%p, Identify = 0x%p", &IdentifyPage, Identify);
	// log_debug(MODULE, "After mallocing ATA_Identify %u, file %s:%u", IdentifyPage.prosses, __FILE__, __LINE__);
	device_t *dev = (device_t *)malloc(sizeof(device_t), ATAPage);
	ide_private_data *priv = (ide_private_data *)malloc(sizeof(ide_private_data), ATAPage);

	// log_debug(MODULE, "After mallocing others, file %s:%u", __FILE__, __LINE__);
	// log_debug(MODULE, "ATA Mod %s", ide_devices[0].Model);

	priv->drive = (ATA_PRIMARY << 1) | ATA_MASTER;

	char *temp = ide_devices[0].Model;

	// log_warn(MODULE, "IdentifyPage.prosses: %u, file %s:%u", IdentifyPage.prosses, __FILE__, __LINE__);

	dev->name = temp;
	dev->id = 32;
	dev->dev_type = DEVICE_BLOCK;
	dev->priv = priv;
	dev->read = ATA_read;
	ata_Device = addDevice(dev);
}

bool ATA_identify(IdentifyDeviceData *buffer)
{
	uint16_t channel = 0;

	ide_select_drive(ATA_PRIMARY, ATA_MASTER);

	ide_write(ATA_PRIMARY, ATA_REG_SECCOUNT0, 0);
	ide_write(ATA_PRIMARY, ATA_REG_LBA0, 0);
	ide_write(ATA_PRIMARY, ATA_REG_LBA1, 0);
	ide_write(ATA_PRIMARY, ATA_REG_LBA2, 0);
	ide_write(ATA_PRIMARY, ATA_REG_COMMAND, ATA_CMD_IDENTIFY);

	ATA_waitBusy();
	if (ATA_CheakError(ATA_PRIMARY))
	{
		printf("ERR set, device failure!\n");
		panic();
		return false;
	}

	ide_read_buffer(ATA_PRIMARY, ATA_REG_DATA, (uint32_t)buffer, 128);
	return true;
}
