#include "ATA.h"
#include "arch/i686/vga_text.h"
#include "hal/hal.h"
#include "memory.h"
#include "arch/i686/io.h"
#include "arch/i686/i8259.h"
#include "stdio.h"
#include "arch/i686/idt.h"
#include "arch/i686/gdt.h"
#include "fs/disk.h"
#include "debug.h"
#include "malloc.h"
#include "arch/i686/isr.h"
#include "arch/i686/irq.h"
#include "drivers/ide/ide_controller.h"

#include "common.h"

#include <stdint.h>

#define MODULE "ATA"

#define ATA_PRIMARY_IO (uint16_t)0x1F0
#define ATA_SECONDARY_IO (uint16_t)0x170

#define ATA_PRIMARY_DCR_AS 0x3F6
#define ATA_SECONDARY_DCR_AS 0x376

#define ATA_PRIMARY_IRQ 46
#define ATA_SECONDARY_IRQ 47

#define ATA_MasterDrive 0xA0
#define ATA_SlaveDirve 0xB0

uint8_t ATA_CheakStatus(uint16_t io, uint8_t flag)
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
#define ATA_waitBusy(io)                                           \
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

uint8_t ata_pm = 0; /* Primary master exists? */
uint8_t ata_ps = 0; /* Primary Slave exists? */
uint8_t ata_sm = 0; /* Secondary master exists? */
uint8_t ata_ss = 0; /* Secondary slave exists? */

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

uint8_t ahci_wait(uint32_t base_address, uint32_t mask, uint32_t desired_result, uint32_t time)
{
	/*
	ticks = 0;
	while (ticks < time)
	{
		asm("nop");
		if ((mmio_ind(base_address) & mask) == desired_result)
		{
			return STATUS_GOOD;
		}
	}
	if (ticks >= time)
	{ // error
	return STATUS_ERROR;
	}
	*/
	return STATUS_ERROR;
}

uint8_t ahci_send_command(uint32_t port_base_address, uint32_t command_list_memory, uint8_t command_type, uint8_t command_direction, uint8_t atapi_command[0x10], uint8_t command, uint16_t sector_count, uint32_t lba, uint32_t memory, uint32_t byte_count)
{
	// wait for possibility to send command
	if (ahci_wait(port_base_address + 0x20, 0x88, 0x0, 200) == STATUS_ERROR)
	{
		log_debug(MODULE, "\nAHCI ERROR: BSY and DRQ are not clear %X %X %X", i686_ind(port_base_address + 0x10), i686_ind(port_base_address + 0x20), i686_ind(port_base_address + 0x30));
		return STATUS_ERROR;
	}

	// clear all memory involved
	// clear_memory(command_list_memory, 0x2000);

	// set command list entry 0
	ahci_command_list_entry_t *ahci_command_list_entry = (ahci_command_list_entry_t *)(command_list_memory);
	ahci_command_list_entry->command_fis_length_in_dwords = 5;
	ahci_command_list_entry->atapi = command_type;
	ahci_command_list_entry->write = command_direction;
	ahci_command_list_entry->number_of_command_table_entries = 1;
	ahci_command_list_entry->command_table_low_memory = (command_list_memory + 0x1000);

	// set FIS command
	ahci_command_and_prd_table_t *ahci_command_and_prd_table = (ahci_command_and_prd_table_t *)(command_list_memory + 0x1000);
	ahci_command_and_prd_table->fis_type = 0x27;
	ahci_command_and_prd_table->flags = 0x80;
	ahci_command_and_prd_table->command = command;
	ahci_command_and_prd_table->lba_0 = ((lba >> 0) & 0xFF);
	ahci_command_and_prd_table->lba_1 = ((lba >> 8) & 0xFF);
	ahci_command_and_prd_table->lba_2 = ((lba >> 16) & 0xFF);
	ahci_command_and_prd_table->device_head = 0xE0;
	ahci_command_and_prd_table->lba_3 = ((lba >> 24) & 0xFF);
	ahci_command_and_prd_table->sector_count_low = ((sector_count >> 0) & 0xFF);
	ahci_command_and_prd_table->sector_count_high = ((sector_count >> 8) & 0xFF);
	ahci_command_and_prd_table->control = 0x08;

	// set ATAPI command
	if (command_type == AHCI_ATAPI)
	{
		for (uint32_t i = 0; i < 0x10; i++)
		{
			ahci_command_and_prd_table->atapi_command[i] = atapi_command[i];
		}
	}

	// set PRD table
	if (byte_count != 0)
	{
		ahci_command_and_prd_table->data_base_low_memory = memory;
		ahci_command_and_prd_table->data_byte_count = (byte_count - 1);
	}

	// wait to be possible to send command list entry 0
	if (ahci_wait(port_base_address + 0x38, 0x01, 0x0, 100) == STATUS_ERROR)
	{
		log_debug(MODULE, "\nAHCI ERROR: command 0 is not free");
		return STATUS_ERROR;
	}

	// clear error register
	i686_outd(port_base_address + 0x10, 0xFFFFFFFF);

	// send command list entry 0
	i686_outd(port_base_address + 0x38, 0x01);

	// wait for transfer outcome
	/*
	ticks = 0;
	while (ticks < 2000)
	{ // wait max 4 seconds
	asm("nop");
		if ((i686_ind(port_base_address + 0x38) & 0x1) == 0x0)
		{ // command was processed
			if ((i686_ind(port_base_address + 0x10) & 0x40000000) == 0x0)
			{ // check for error
				return STATUS_GOOD;
			}
			else
			{
				return STATUS_ERROR;
			}
		}
		if ((i686_ind(port_base_address + 0x10) & 0x40000000) == 0x40000000)
		{ // error
			// we need to clear COMMAND_ISSUE register to be able to send commands again, to do so we will restart command list
			i686_outd(port_base_address + 0x18, i686_ind(port_base_address + 0x18) & ~0x01); // clear start bit
			if (ahci_wait(port_base_address + 0x18, 0x1, 0x0, 100) == STATUS_ERROR)
			{
				log_debug(MODULE, "\nAHCI ERROR: port command list can not be stopped");
			}
			i686_outd(port_base_address + 0x18, i686_ind(port_base_address + 0x18) | 0x01); // set start bit
			if (ahci_wait(port_base_address + 0x18, 0x1, 0x1, 100) == STATUS_ERROR)
			{
				log_debug(MODULE, "\nAHCI ERROR: port command list can not be started");
			}

			return STATUS_ERROR;
		}
	}
	*/

	// timeout
	log_debug(MODULE, "\nAHCI: command timeout");
	return STATUS_ERROR;
}

void ide_select_drive(uint8_t ioBase, uint8_t drive)
{
	if (ioBase == ATA_PRIMARY)
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
	uint8_t status = i686_inb(ATA_PRIMARY_IO + ATA_REG_STATUS);
	if ((status & ATA_SR_ERR) == ATA_SR_ERR)
	{
		ide_print_error(ATA_PRIMARY, 2);
	}

	i8259_SendEndOfInterrupt(ATA_PRIMARY_IRQ);
}

void ide_secondary_irq(Registers *regs)
{
	uint8_t status = i686_inb(ATA_SECONDARY_IO + ATA_REG_STATUS);
	if ((status & ATA_SR_ERR) == ATA_SR_ERR)
	{
		ide_print_error(ATA_SECONDARY, 2);
	}

	i8259_SendEndOfInterrupt(ATA_SECONDARY_IRQ);
}

void ide_poll(uint16_t ioBase)
{
	ATA_waitBusy(ioBase);
	if (ATA_CheakError(ioBase))
	{
		printf("ERR set, device failure!\n");
		panic();
	}
	return;
}

uint32_t ata_read_one(uint8_t *buf, uint32_t lba, uint32_t offset, uint16_t device)
{
	lba &= 0x00FFFFFF; // ignore topmost byte
	/* We only support 28bit LBA so far */
	uint16_t channel = 0;
	uint16_t drive = 0;
	ATA_GetIO(device, &channel, &drive);
	uint8_t cmd = (drive == ATA_MASTER ? 0xE0 : 0xF0);
	uint8_t slavebit = (drive == ATA_MASTER ? 0x00 : 0x01);
	/*
	kprintf("LBA = 0x%x\n", lba);
	kprintf("LBA>>24 & 0x0f = %d\n", (lba >> 24)&0x0f);
	kprintf("(uint8_t)lba = %d\n", (uint8_t)lba);
	kprintf("(uint8_t)(lba >> 8) = %d\n", (uint8_t)(lba >> 8));
	kprintf("(uint8_t)(lba >> 16) = %d\n", (uint8_t)(lba >> 16));
	*/

	/*
				   mov edx, 0x01F6      ; Port to send drive and bit 24 - 27 of LBA
				   shr eax, 24          ; Get bit 24 - 27 in al
				   or al, 11100000b     ; Set bit 6 in al for LBA mode
				   out dx, al

				   mov edx, 0x01F2      ; Port to send number of sectors
				   mov al, cl           ; Get number of sectors from CL
				   out dx, al

				   mov edx, 0x1F3       ; Port to send bit 0 - 7 of LBA
				   mov eax, ebx         ; Get LBA from EBX
				   out dx, al

				   mov edx, 0x1F4       ; Port to send bit 8 - 15 of LBA
				   mov eax, ebx         ; Get LBA from EBX
				   shr eax, 8           ; Get bit 8 - 15 in AL
				   out dx, al


				   mov edx, 0x1F5       ; Port to send bit 16 - 23 of LBA
				   mov eax, ebx         ; Get LBA from EBX
				   shr eax, 16          ; Get bit 16 - 23 in AL
				   out dx, al

				   mov edx, 0x1F7       ; Command port
				   mov al, 0x20         ; Read with retry.
				   out dx, al
	 */

	// outportb(io + ATA_REG_HDDEVSEL, cmd | ((lba >> 24)&0x0f));

	i686_outb(channel + ATA_REG_HDDEVSEL, (cmd | (uint8_t)((lba >> 24 & 0x0F))));
	// mprint("issued 0x%x to 0x%x\n", (cmd | (lba >> 24)&0x0f), channel + ATA_REG_HDDEVSEL);
	// for(int k = 0; k < 10000; k++) ;
	i686_outb(channel + 1, 0x00);
	// mprint("issued 0x%x to 0x%x\n", 0x00, channel + 1);
	// for(int k = 0; k < 10000; k++) ;
	i686_outb(channel + ATA_REG_SECCOUNT0, 1);
	// mprint("issued 0x%x to 0x%x\n", (uint8_t) numsects, channel + ATA_REG_SECCOUNT0);
	// for(int k = 0; k < 10000; k++) ;
	i686_outb(channel + ATA_REG_LBA0, (uint8_t)((lba)));
	// mprint("issued 0x%x to 0x%x\n", (uint8_t)((lba)), channel + ATA_REG_LBA0);
	// for(int k = 0; k < 10000; k++) ;
	i686_outb(channel + ATA_REG_LBA1, (uint8_t)((lba) >> 8));
	// mprint("issued 0x%x to 0x%x\n", (uint8_t)((lba) >> 8), channel + ATA_REG_LBA1);
	// for(int k = 0; k < 10000; k++) ;
	i686_outb(channel + ATA_REG_LBA2, (uint8_t)((lba) >> 16));
	// mprint("issued 0x%x to 0x%x\n", (uint8_t)((lba) >> 16), channel + ATA_REG_LBA2);
	// for(int k = 0; k < 10000; k++) ;
	i686_outb(channel + ATA_REG_COMMAND, ATA_CMD_READ_PIO);
	// mprint("issued 0x%x to 0x%x\n", ATA_CMD_READ_PIO, channel + ATA_REG_COMMAND);

	ATA_CheakStatus(channel, ATA_SR_DRQ);

	ide_poll(channel);

	uint32_t count = 0;
	// set_task(0);
	for (int i = 0; i < 256; i++)
	{
		count++;
		uint16_t data = i686_inw(channel + ATA_REG_DATA);
		uint8_t b1 = data & 0x00FF;
		uint8_t b2 = (data >> 8);

		buf[(i * 2) + offset] = b1;
		buf[(i * 2) + offset + 1] = b2;
		// log_debug(MODULE, "%X, %X", b1, b2);
	}
	ide_400ns_delay(channel);
	// set_task(1);
	return count;
}

uint32_t ATA_read(uint8_t *buf, uint32_t lba, uint32_t numsects, uint16_t device)
{
	uint32_t count = 0;
	uint32_t offset = 0;
	for (int i = 0; i < numsects; i++)
	{
		count += ata_read_one(buf, lba + i, offset, device);
		offset += 512;
	}
	return count * 2;
}

void ATA_init()
{
	ide_initialize(0x1F0, 0x3F6, 0x170, 0x376, 0x000);
	printf("Checking for ATA drives\n");
	i686_IRQ_RegisterHandler(ATA_PRIMARY_IRQ, ide_primary_irq);
	i686_IRQ_RegisterHandler(ATA_SECONDARY_IRQ, ide_secondary_irq);

	filesystemInfo_t *fs = (filesystemInfo_t*)malloc(sizeof(filesystemInfo_t));
	fs->name = "ATA";

	device_t * dev = (device_t*)malloc(sizeof(device_t)); 

}

bool ATA_identify(ATA_Identify_t *buffer)
{
	uint16_t channel = 0;
	ide_select_drive(ATA_PRIMARY, ATA_MASTER);
	ATA_GetIO(ATA_PRIMARY, &channel, NULL);

	i686_outb(channel + ATA_REG_SECCOUNT0, 0);
	i686_outb(channel + ATA_REG_LBA0, 0);
	i686_outb(channel + ATA_REG_LBA1, 0);
	i686_outb(channel + ATA_REG_LBA2, 0);
	i686_outb(channel + ATA_REG_COMMAND, ATA_CMD_IDENTIFY);

	ATA_waitBusy(channel);
	if (ATA_CheakError(channel))
	{
		printf("ERR set, device failure!\n");
		panic();
		return false;
	}

	uint8_t *pBuffer = ((uint8_t *)buffer);
	for (int i = 0; i < 256; i++)
	{
		uint16_t data = i686_inw(channel + ATA_REG_DATA);
		uint8_t b1 = data & 0x00FF;
		uint8_t b2 = (data >> 8);

		pBuffer[i * 2] = b1; 
		pBuffer[i * 2 + 1] = b2; 
	}
	return true;
}
