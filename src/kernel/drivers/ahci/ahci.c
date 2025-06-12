#include "ahci.h"
#include "drivers/ATA/ATA.h"

#include "drivers/ide/ide_controller.h"
#include "debug.h"
#include "arch/i686/io.h"
#include "arch/i686/i8259.h"

#include "memory.h"
#include "malloc.h"

#define MODULE "AHCI"

#define SATA_SIG_ATA 0x00000101	  // SATA drive
#define SATA_SIG_ATAPI 0xEB140101 // SATAPI drive
#define SATA_SIG_SEMB 0xC33C0101  // Enclosure management bridge
#define SATA_SIG_PM 0x96690101	  // Port multiplier

/*
Enable interrupts, DMA, and memory space access in the PCI command register
Memory map BAR 5 register as uncacheable.
Perform BIOS/OS handoff (if the bit in the extended capabilities is set)
Reset controller
Register IRQ handler, using interrupt line given in the PCI register. This interrupt line may be shared with other devices, so the usual implications of this apply.
Enable AHCI mode and interrupts in global host control register.
Read capabilities registers. Check 64-bit DMA is supported if you need it.
For all the implemented ports:
	Allocate physical memory for its command list, the received FIS, and its command tables. Make sure the command tables are 128 byte aligned.
	Memory map these as uncacheable.
	Set command list and received FIS address registers (and upper registers, if supported).
	Setup command list entries to point to the corresponding command table.
	Reset the port.
	Start command list processing with the port's command register.
	Enable interrupts for the port. The D2H bit will signal completed commands.
	Read signature/status of the port to see if it connected to a drive.
	Send IDENTIFY ATA command to connected drives. Get their sector size and count.
*/

typedef struct
{
	HBAData *abar;
	HBAPort *port;
	void *clb;
	void *fb;
	void *ctba[32];
	void *unused[28]; // Even out the data size to 256 bytes
} ahci_port;

Page *ahciPages;
ahci_port *ports;
int num_ports;

uint32_t find_cmdslot(ahci_port aport)
{
	HBAPort *port = aport.port;
	uint32_t slots = (port->sact | port->ci);
	uint32_t cmdslots = (aport.abar->cap & 0x0f00) >> 8;
	for (uint32_t i = 0; i < cmdslots; i++)
	{
		if ((slots & 1) == 0)
			return i;
		slots >>= 1;
	}
	return 0xFFFFFFFF;
}

uint8_t ahci_identify_device(ahci_port aport, void *buf)
{
	HBAPort *port = aport.port;
	port->is = 0xFFFFFFFF;
	uint32_t slot = find_cmdslot(aport);
	if (slot == 0xFFFFFFFF)
		return 1;

	HBA_CMD_HEADER *cmdheader = (HBA_CMD_HEADER *)aport.clb;
	cmdheader += slot;
	cmdheader->cfl = sizeof(FIS_REG_H2D) / sizeof(uint32_t);
	cmdheader->w = 0;

	HBA_CMD_TBL *cmdtable = (HBA_CMD_TBL *)aport.ctba[slot];

	cmdtable->prdt_entry[0].dba = (uint32_t)buf;
	cmdtable->prdt_entry[0].dbau = 0;
	cmdtable->prdt_entry[0].dbc = 511;
	cmdtable->prdt_entry[0].i = 1;

	FIS_REG_H2D *cmdfis = (FIS_REG_H2D *)(&cmdtable->cfis);

	cmdfis->fis_type = FIS_TYPE_REG_H2D; // Host to device
	cmdfis->c = 1;
	cmdfis->command = SATA_IDENTIFY_DEVICE;

	cmdfis->device = 1 << 6;

	for (uint32_t spin = 0; spin < 1000000; spin++)
	{
		if (!(port->tfd & (SATA_BUSY | SATA_DRQ)))
			break;
	}
	if ((port->tfd & (SATA_BUSY | SATA_DRQ)))
		return 2;

	port->ci = (1 << slot);

	while (1)
	{
		if (!(port->ci & (1 << slot)))
			break;
		if (port->is & (1 << 30))
			return 3;
	}

	if (port->is & (1 << 30))
		return 3;

	return 0;
}

void initialize_port(ahci_port *aport)
{
	HBAPort *port = aport->port;
	port->cmd &= ~HBA_CMD_ST;
	port->cmd &= ~HBA_CMD_FRE;

	while ((port->cmd & HBA_CMD_FR) || (port->cmd & HBA_CMD_CR))
		;

	void *mapped_clb = malloc(PAGE_SIZE, ahciPages);
	memset(mapped_clb, 0, 4096);
	port->clb = (uint32_t)mapped_clb;
	port->clbu = 0;
	aport->clb = mapped_clb;

	void *mapped_fb = malloc(PAGE_SIZE, ahciPages);
	memset(mapped_fb, 0, 4096);
	port->fb = (uint32_t)mapped_fb;
	port->fbu = 0;
	aport->fb = mapped_fb;

	HBA_CMD_HEADER *cmdheader = (HBA_CMD_HEADER *)mapped_clb;

	for (uint8_t i = 0; i < 32; i++)
	{
		cmdheader[i].prdtl = 1;
		void *ctba_buf = calloc(1, PAGE_SIZE, ahciPages);
		aport->ctba[i] = ctba_buf;
		cmdheader[i].ctba = (uint32_t)ctba_buf;
		cmdheader[i].ctbau = 0;
	}

	while (port->cmd & HBA_CMD_CR)
		;
	port->cmd |= HBA_CMD_FRE;
	port->cmd |= HBA_CMD_ST;
}

bool is_sata(HBAPort *port)
{
	uint8_t ipm = (port->ssts >> 8) & 0xF;
	uint8_t det = (port->ssts) & 0xF;
	if (det != HBA_DET_PRESENT || ipm != HBA_IPM_ACTIVE)
		return false;
	return true;
}

void InitAbar(HBAData *abar, device_t* dev)
{
	uint32_t pi = abar->pi;
	for (uint8_t i = 0; i < 32; i++)
	{
		if (pi & 1)
		{
			if (is_sata(&abar->ports[i]))
			{
				memset(&ports[num_ports], 0, 256);
				ports[num_ports].abar = abar;
				ports[num_ports].port = &abar->ports[i];
				initialize_port(&ports[num_ports]);
				sata_identify_packet info;
				ahci_identify_device(ports[num_ports], &info);
				ide_private_data* priv = dev->priv;
				priv->drive = num_ports;
				log_debug(MODULE, "Port %u initialized", num_ports);
				char name[41] = {0};
				for (int i = 0; i < 40; i += 2)
				{
					name[i] = info.model_number[i + 1];
					name[i + 1] = info.model_number[i];
				}
				dev->name = name;
				printf("Detected SATA drive: %s (%u MiB)\n", name, info.total_sectors / 2048);
				log_debug(MODULE, "Detected SATA drive: %s (%u MiB)", name, info.total_sectors / 2048);
				num_ports++;
			}
		}
		pi >>= 1;
	}
}

bool ahci_read_sectors_internal(ahci_port aport, uint32_t startl, uint32_t starth, uint32_t count, uint16_t *buf)
{
	HBAPort *port = aport.port;
	port->is = (uint32_t)-1; // Clear pending interrupt bits
	uint32_t slot = find_cmdslot(aport);

	if (slot == -1)
		return false;

	HBA_CMD_HEADER *cmdheader = (HBA_CMD_HEADER *)aport.clb;
	cmdheader += slot;
	cmdheader->cfl = sizeof(FIS_REG_H2D) / sizeof(uint32_t); // Command FIS size
	cmdheader->w = 0;										 // Read from device

	HBA_CMD_TBL *cmdtbl = (HBA_CMD_TBL *)(aport.ctba[slot]);

	cmdtbl->prdt_entry[0].dba = (uint32_t)buf;
	cmdtbl->prdt_entry[0].dbc = (count * 512) - 1; // 8K bytes (this value should always be set to 1 less than the actual value)
	cmdtbl->prdt_entry[0].dbau = 0;
	cmdtbl->prdt_entry[0].i = 1;

	// Setup command
	FIS_REG_H2D *cmdfis = (FIS_REG_H2D *)(&cmdtbl->cfis);

	cmdfis->fis_type = FIS_TYPE_REG_H2D;
	cmdfis->c = 1; // Command
	cmdfis->command = ATA_CMD_READ_DMA_EXT;

	cmdfis->lba0 = (uint8_t)startl;
	cmdfis->lba1 = (uint8_t)(startl >> 8);
	cmdfis->lba2 = (uint8_t)(startl >> 16);
	cmdfis->device = 1 << 6;

	cmdfis->lba3 = (uint8_t)(startl >> 24);
	cmdfis->lba4 = (uint8_t)(starth);
	cmdfis->lba5 = (uint8_t)(starth >> 8);

	cmdfis->countl = (count & 0xFF);
	cmdfis->counth = (count >> 8);

	for (uint32_t spin = 0; spin < 1000000; spin++)
	{
		if (!(port->tfd & (SATA_BUSY | SATA_DRQ)))
		{
			break;
		}
	}
	if ((port->tfd & (SATA_BUSY | SATA_DRQ)))
	{
		return 2;
	}

	port->ci = 1 << slot; // Issue command

	while (1)
	{
		if (!(port->ci & (1 << slot)))
			break;
		if (port->is & (1 << 30))
			return 3;
	}

	if (port->is & (1 << 30))
		return 3;

	return true;
}

uint32_t ahci_read_sectors(void *buf, uint64_t start_sector, uint32_t count, device_t* device)
{
	uint32_t drive_num = ((ide_private_data*)(device->priv))->drive;
	if (ports[drive_num].abar != 0)
	{
		uint16_t* u16Buffer = (uint16_t*)buf;
		if (ahci_read_sectors_internal(ports[drive_num], start_sector & 0xFFFFFFFF, (start_sector >> 32) & 0xFFFFFFFF, count, u16Buffer))
		{
			return count;
		}
	}
	else
	{
		return 4;
	}
	return 5;
}
uint16_t AHCI_DeviceIndex;
void AHCI_init(uint32_t bar5)
{
	ahciPages = allocate_page();
	ports = (ahci_port *)malloc(sizeof(ahci_port) * 4, ahciPages);
	device_t *dev = (device_t *)malloc(sizeof(device_t), devicePage);
	ide_private_data *priv = (ide_private_data *)malloc(sizeof(ide_private_data), privPage);

	dev->priv = priv;
	log_crit(MODULE, "ahciPages: %i", ahciPages->prosses);
	
	InitAbar((HBAData *)bar5, dev);

	dev->id = 32;
	ATA_DeviceIndex = 33;
	
	dev->dev_type = DEVICE_BLOCK;	
	dev->read = ahci_read_sectors;

	addDevice(dev);
	log_crit(MODULE, "exiting init");
}
