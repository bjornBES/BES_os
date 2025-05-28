#include "fat32.h"

#include "stdlib.h"
#include "malloc.h"
#include "memory.h"
#include "string.h"
#include "stdio.h"
#include "debug.h"
#include "ctype.h"

#include "math.h"
#include "drivers/Keyboard/keyboard.h"

#define MODULE "FAT32"

#define BOOTSECTOR FatData->BS.BootSector

#define GETCLUSTER(High, Low) ((High << 16) | (Low))

#define GETSECTORWC(High, Low) FatData->FirstDataSector + ((GETCLUSTER(High, Low) - 2) * BOOTSECTOR.SectorsPerCluster)
#define GETSECTOR(Cluster) FatData->FirstDataSector + ((Cluster - 2) * BOOTSECTOR.SectorsPerCluster)

#define FATREADFAT(FATsector, dev) dev->read(FatData->FatCache, FATsector, FAT_CACHE_SIZE, dev)

FAT_Data *FatData = 0;

Page *FATInitDataPage = NULL;
Page *fat32PrivPage = NULL;

void getLFNBlock(FAT_LongFileEntry *entry, FAT_LFNBlock *block)
{
	block->Order = entry->Order;
	memcpy(block->Chars, entry->name1, 5 * sizeof(uint16_t));
	memcpy(block->Chars + 5, entry->name2, 6 * sizeof(uint16_t));
	memcpy(block->Chars + 11, entry->name3, 2 * sizeof(uint16_t));
}

// converts the name into the 8.3 FAT format
void getShortName(char *name, char *shortName)
{
	memset(shortName, ' ', 12);
	shortName[11] = '\0';

	const char *ext = strchr(name, '.');
	if (ext == NULL)
	{
		ext = name + 11;
	}

	for (int i = 0; i < 8 && name[i] && name + i < ext; i++)
	{
		shortName[i] = toupper(name[i]);
	}

	if (ext != name + 11)
	{
		for (int i = 0; i < 3 && ext[i + 1]; i++)
		{
			shortName[i + 8] = toupper(ext[i + 1]);
		}
	}
}

bool FAT_ReadSectors(void *buf, uint32_t sector, uint32_t length, device_t *dev, fatPrivData *priv)
{
	log_debug(MODULE, "file %s:%u", __FILE__, __LINE__);
	return dev->read(buf, sector, length, dev);
}
bool FAT_WriteSectors(void *buf, uint32_t sector, uint32_t length, device_t *dev, fatPrivData *priv)
{
	log_debug(MODULE, "file %s:%u", __FILE__, __LINE__);
	return dev->write(buf, sector, length, dev);
}

uint32_t FAT_Read(void* buf, uint32_t sector, uint32_t byteCount, device_t *dev, fatPrivData *priv)
{
	uint8_t* result = (uint8_t*)buf;

	uint32_t bytesRead = 0;
	uint32_t count = 0;
	while (byteCount > bytesRead)
	{
		uint32_t leftToRead = min(SECTOR_SIZE, byteCount - bytesRead);
		uint32_t take = min(byteCount, leftToRead);

		log_debug(MODULE, "bytesRead: %u - leftToRead: %u - take: %u - ", bytesRead, leftToRead, take);

		FAT_ReadSectors(result, sector + count, 1, dev, priv);
		result += take;
		bytesRead += take;
		count++;
	}
	
	return bytesRead;
}

bool FAT_ReadEntry(FAT_FileEntry* dirEntry, uint32_t sector, device_t *dev, fatPrivData *priv)
{
	return FAT_Read(dirEntry, sector, sizeof(FAT_FileEntry), dev, priv) == sizeof(FAT_FileEntry);
}

uint32_t FAT_GetFatEOF()
{
	if (FatData->FATType == FAT12)
	{
		return 0xFF8;
	}
	else if (FatData->FATType == FAT16)
	{
		return 0xFFF8;
	}
	else if (FatData->FATType == FAT32)
	{
		return 0xFFFFFFF8;
	}

	return 0;
}

void FAT_GetDir(FAT_DirectoryEntry *entry, FAT_Directory *dir, uint8_t *buffer, device_t *dev, fatPrivData *priv)
{
	log_debug(MODULE, "file %s:%u", __FILE__, __LINE__);
	uint32_t firstSector = GETSECTORWC(entry->FirstClusterHigh, entry->FirstClusterLow);
	FAT_ReadSectors(buffer, firstSector, BOOTSECTOR.SectorsPerCluster, dev, priv);
	dir->entries = (FAT_FileEntry *)buffer;
}

uint32_t FAT_NextCluster(uint32_t currentCluster, device_t *dev)
{
	uint32_t fatIndex;
	if (FatData->FATType == FAT12)
	{
		fatIndex = currentCluster * 3 / 2;
	}
	else if (FatData->FATType == FAT16)
	{
		fatIndex = currentCluster * 2;
	}
	else if (FatData->FATType == FAT32)
	{
		fatIndex = currentCluster * 4;
	}

	log_info(MODULE, "BytesPerSector %u", BOOTSECTOR.BytesPerSector);
	log_info(MODULE, "fatIndex %u", fatIndex);
	uint32_t fatIndexSector = fatIndex / BOOTSECTOR.BytesPerSector;
	if (fatIndexSector < FatData->FatCachePosition || fatIndexSector >= FatData->FatCachePosition + FAT_CACHE_SIZE)
	{
		FATREADFAT(fatIndexSector, dev);
		FatData->FatCachePosition = fatIndexSector;
	}

	fatIndex -= FatData->FatCachePosition * BOOTSECTOR.BytesPerSector;
	uint32_t nextCluster;
	if (FatData->FATType == FAT12)
	{
		if (currentCluster % 2 == 0)
		{
			nextCluster = (*(uint16_t *)(FatData->FatCache + fatIndex)) & 0x0FFF;
		}
		else
		{
			nextCluster = (*(uint16_t *)(FatData->FatCache + fatIndex)) >> 4;
		}

		if (nextCluster >= 0xFF8)
		{
			nextCluster |= 0xFFFFF000;
		}
	}
	else if (FatData->FATType == FAT16)
	{
		nextCluster = *(uint16_t *)(FatData->FatCache + fatIndex);
		if (nextCluster >= 0xFFF8)
		{
			nextCluster |= 0xFFFF0000;
		}
	}
	else if (FatData->FATType == FAT32)
	{
		nextCluster = *(uint32_t *)(FatData->FatCache + fatIndex);
	}

	return nextCluster;
}

bool FAT_ReadClusters(uint8_t *buf, uint32_t firstCluster, uint32_t size, device_t *dev, fatPrivData *priv)
{
	uint32_t FatEOF = FAT_GetFatEOF();
	uint32_t cluster = firstCluster;
	uint32_t bytesRead = 0;
	while (cluster < FatEOF && bytesRead < size)
	{
		uint32_t sector = GETSECTOR(cluster);

		FAT_ReadSectors(buf + bytesRead, sector, BOOTSECTOR.SectorsPerCluster, dev, priv);

		bytesRead += priv->BytesPerCluster;
		cluster = FAT_NextCluster(cluster, dev);
	}
	return true;
}

int FAT_CompareLFNBlocks(const void *blockA, const void *blockB)
{
	FAT_LFNBlock *a = (FAT_LFNBlock *)blockA;
	FAT_LFNBlock *b = (FAT_LFNBlock *)blockB;
	return ((int)a->Order) - ((int)b->Order);
}

void FAT_GetLfn(FAT_LongFileEntry *lfnEntries, int entryCount, char *output)
{
	log_debug(MODULE, "file %s:%u", __FILE__, __LINE__);
	if (entryCount <= 0 || !lfnEntries || !output)
	{
		log_err("FAT32 LONG", "Exit error: entryCount: %u ", entryCount);
		*output = '\0';
		return;
	}

	// Sort LFN entries by Order (increasing)
	qsort(lfnEntries, entryCount, sizeof(FAT_LongFileEntry), FAT_CompareLFNBlocks);

	Page LFNBlockPage;
	FAT_LFNBlock *LFNBlock = (FAT_LFNBlock *)malloc(sizeof(FAT_LFNBlock), &LFNBlockPage);
	char *outPtr = output;
	for (int i = 0; i < entryCount; i++)
	{
		FAT_LongFileEntry *entry = &lfnEntries[i];

		getLFNBlock(entry, LFNBlock);
		int16_t *utf16_chars = LFNBlock->Chars;

		for (int j = 0; j < 13; j++)
		{
			if (utf16_chars[j] == 0xFFFF || utf16_chars[j] == 0x0000)
			{
				*outPtr = '\0';
				free(LFNBlock, &LFNBlockPage);
				return; // End of name
			}

			int codepoint;
			utf16_to_codepoint((uint16_t *)&utf16_chars[j], &codepoint);
			outPtr = codepoint_to_utf8(codepoint, outPtr);
		}
	}

	*outPtr = '\0'; // Null-terminate the string
	free(LFNBlock, &LFNBlockPage);
	return;
}

uint32_t fat32GetRootCount(device_t *dev)
{
	uint32_t result = 0;
	uint32_t currentFat = FAT_NextCluster(0, dev);

	while (currentFat < FAT32_EOC)
	{
		result++;

		if (currentFat == 0)
			break; // Error or end
		currentFat = FAT_NextCluster(currentFat, dev);
	}

	return result;
}

bool FAT_ReadDirectory(char *filename, FAT_Directory *dir, device_t *dev, fatPrivData *priv)
{
	// Parse the buffer to get directory entries
	uint32_t entryOffset = 0;
	uint16_t index = 0;
	dir->entryCount = 0;
	while (entryOffset < SECTOR_SIZE)
	{
		FAT_DirectoryEntry entry = dir->entries[index].Entry;
		// Check if the entry is empty (which marks the end of the directory)

		if (entry.Name[0] == 0x00)
		{
			break;
		}
		if (entry.Name[0] == '\0')
		{
			break;
		}

		// log_debug(MODULE, "entry: %s %X", entry.Name, entry.Attributes); // for debuging things
		Page namePage;
		char *name = 0;
		if (entry.Name[0] == 0xE5)
		{
			continue;
		}
		if (entry.Attributes != FAT_ATTRIBUTE_LFN)
		{
			name = (char *)malloc(12, &namePage);
			memset(name, ' ', 12);
			memcpy(name, entry.Name, 11);
		}
		else
		{
			name = (char *)malloc(256, &namePage);
			memset(name, 0, 256);
			FAT_DirectoryEntry entry = dir->entries[index].Entry;

			// Check for long file names (LFN entries)
			if (entry.Attributes == FAT_ATTRIBUTE_LFN)
			{
				FAT_LongFileEntry *LongEntry = &dir->entries[index].LongEntry;
				uint32_t LFNCount = (uint32_t)LongEntry->Order & ~FAT_LFN_LAST;
				FAT_LongFileEntry NLFentries[LFNCount];
				for (size_t i = 0; i < LFNCount; i++)
				{
					NLFentries[i] = dir->entries[index].LongEntry;
					index++;
					entryOffset += sizeof(FAT_DirectoryEntry);
				}

				FAT_GetLfn(NLFentries, LFNCount, name);

				entryOffset += sizeof(FAT_DirectoryEntry);
				index++;

				entry = dir->entries[index].Entry;
			}
		}
		if (name[0] == '.')
		{
			// Move to the next directory entry
			if ((uint32_t)filename != 1)
			{
				dir->entryCount++;
			}
			entryOffset += sizeof(FAT_DirectoryEntry);
			index++;
			continue;
		}

		log_info(MODULE, "--entry: %s", name);
		// If no filename is provided, print out the directory entry
		if (!filename && (uint32_t)filename != 1)
		{
			log_info(MODULE, "Found dir entry: %s", name);
		}
		else
		{
			// If filename is given, compare it with the directory entry's name
			if (filename && strcmp(filename, name) == 0)
			{
				uint32_t firstCluster = GETCLUSTER(entry.FirstClusterHigh, entry.FirstClusterLow);
				log_info(MODULE, "Found file %s! FirstCluster: %d", filename, firstCluster);
				free(name, &namePage);
				return firstCluster; // Return true if found
			}
		}

		free(name, &namePage);
		// Move to the next directory entry
		if ((uint32_t)filename == 1)
		{
			dir->entryCount++;
		}
		entryOffset += sizeof(FAT_DirectoryEntry);
		index++;
	}
	// If no match was found, return false or 0
	return false;
}
bool FAT_ReadRootDirectory(char *filename, device_t *dev, fatPrivData *priv)
{
	// Calculate the first sector of the root directory
	uint32_t rootDirSector = FatData->RootDirSector;
	// log_debug(MODULE, "rootDirSector = %u", rootDirSector);

	// Read the first sector of the root directory (this contains the entries)
	// log_debug(MODULE, "root size = %u", priv->RootDirSizeSec);
	Page bufferPage;
	uint8_t *buffer = (uint8_t *)malloc(priv->RootDirSizeSec * (BOOTSECTOR.BytesPerSector * BOOTSECTOR.SectorsPerCluster), &bufferPage);
	if (!buffer)
	{
		free(buffer, &bufferPage);
		return false;
	}
	// log_debug(MODULE, "rootDirSector = %u", rootDirSector / priv->SectorsPerBlock); // for debuging things
	FAT_ReadSectors(buffer, rootDirSector, priv->RootDirSizeSec, dev, priv);
	// log_debug(MODULE, "FatData = %p rootDirSector = %u", FatData, rootDirSector); // for debuging things

	// Parse the buffer to get directory entries
	uint32_t entryOffset = 0;
	uint8_t index = 0;

	FatData->RootDirectory.entries = (FAT_FileEntry *)buffer;
	FAT_Directory *rootDir = &FatData->RootDirectory;
	
	while (entryOffset < rootDirSector * BOOTSECTOR.BytesPerSector)
	{
		FAT_DirectoryEntry entry = rootDir->entries[index].Entry;
		// log_info(MODULE, "      entry: %s 0x%X", entry.Name, entry.Attributes); // for debuging things
		// Check if the entry is empty (which marks the end of the directory)
		if (entry.Name[0] == 0x00)
		{
			break;
		}

		// Move to the next directory entry
		if ((uint32_t)(filename) == 1)
		{
			FatData->RootDirectory.entryCount++;
		}
		entryOffset += sizeof(FAT_DirectoryEntry);
		index++;
	}

	free(buffer, &bufferPage);

	if (filename && (uint32_t)filename != 1)
		return false;
	return true;
}

bool FAT_ReadFile(char *fileName, uint8_t *buffer, device_t *dev, fatPrivData *priv)
{
	if (!fileName || !buffer || !dev || !priv)
	{
		log_crit(MODULE, "Invalid arguments");
		return false;
	}

	if (fileName[0] != '/')
	{
		log_crit(MODULE, "Only absolute paths are supported");
		return false;
	}

	if (!fileName)
	{
		log_crit(MODULE, "fileName is null, Filepath is invalid");
		return false;
	}

	FAT_ReadRootDirectory((char *)1, dev, priv);
	if (FatData->RootDirectory.entryCount == 0)
	{
		log_crit(MODULE, "Root directory has no entries");
		return false;
	}

	char *segment = strtok(fileName, "/");
	log_debug(MODULE, "segment = %s from %s", segment, fileName);
	if (segment == NULL)
	{
		log_crit(MODULE, "Filepath is invalid");
		return false;
	}

	FAT_FileEntry *entry = NULL;
	FAT_Directory currentDir = FatData->RootDirectory;

	while (segment)
	{
		char shortName[13] = {' '};
		getShortName(segment, shortName);
		log_debug(MODULE, "shortName = %s segment = %s", shortName, segment);

		log_debug(MODULE, "currentDir count %u", currentDir.entryCount);

		for (int i = 0; i < currentDir.entryCount; i++)
		{
			FAT_FileEntry *dirEntry = &currentDir.entries[i];
			log_debug(MODULE, "%u: %s 0x%X", i, dirEntry->Entry.Name, dirEntry->Entry.Attributes);

			if ((dirEntry->Entry.Attributes & FAT_ATTRIBUTE_LFN) == 0) // short name 8.3
			{
				char entryName[13] = {' '};
				memcpy(entryName, dirEntry->Entry.Name, 11);

				log_debug(MODULE, "entry name: %s segment: %s", entryName, shortName);

				if (strcmp(shortName, entryName) == 0)
				{
					entry = dirEntry;
					log_info(MODULE, "Found %s", shortName);
					break;
				}
			}
			else
			{
			}
		}

		if (!entry)
		{
			log_crit(MODULE, "File or directory not found: %s", segment);
			return false;
		}

		// If this is the last segment, we've found the file
		if (!strtok(NULL, "/"))
		{
			break;
		}

		// Otherwise, move into the directory
		if (!(entry->Entry.Attributes & FAT_ATTRIBUTE_DIRECTORY))
		{
			log_crit(MODULE, "%s is not a directory", segment);
			return false;
		}
		Page bufferPage;
		uint8_t *buffer = (uint8_t *)malloc(priv->BytesPerCluster, &bufferPage);
		FAT_GetDir(&entry->Entry, &currentDir, buffer, dev, priv);
		segment = strtok(NULL, "/");
		free(buffer, &bufferPage);
	}

	// Read file data
	if (!(entry->Entry.Attributes & FAT_ATTRIBUTE_DIRECTORY))
	{
		uint32_t firstCluster = GETCLUSTER(entry->Entry.FirstClusterHigh, entry->Entry.FirstClusterLow);
		FAT_ReadClusters(buffer, firstCluster, entry->Entry.Size, dev, priv);
		return true;
	}
	return false;
}

bool FAT_Probe(device_t *dev)
{
	log_info(MODULE, "Probing device %d", dev->id);
	if (!dev->read)
	{
		printf("Device has no read, skipped.\n");
		return 0;
	}

	FatData = (FAT_Data *)malloc(sizeof(FAT_Data), FATInitDataPage);
	FatData->FatCachePosition = -1;
	dev->read(FatData->BS.BootSectorBytes, 0, 1, dev);

	// getting the sectors per fat
	if (BOOTSECTOR.SectorsPerFat != 0)
	{
		FatData->FATSize = BOOTSECTOR.SectorsPerFat;
	}
	else
	{
		FatData->FATSize = BOOTSECTOR.EBR32.SectorsPerFat;
	}

	// The starting sector of the Data Area (or the Root Directory sector in FAT12/FAT16)
	uint32_t StartOfDataArea = BOOTSECTOR.ReservedSectors + (BOOTSECTOR.FatCount * FatData->FATSize);

	// Total number of sectors that can fit the root directory entries
	uint32_t RootDirCount;

	// getting the total number of sectors on the DISK
	uint32_t TotSec = 0;
	if (BOOTSECTOR.TotalSectors != 0)
	{
		TotSec = BOOTSECTOR.TotalSectors;
		RootDirCount = ((BOOTSECTOR.DirEntryCount * DIR_ENTRY_SIZE) + (BOOTSECTOR.BytesPerSector - 1)) / BOOTSECTOR.BytesPerSector;
	}
	else
	{
		TotSec = BOOTSECTOR.LargeSectorCount;
		RootDirCount = fat32GetRootCount(dev);
	}

	// FAT12/FAT16: The sector where data starts, immediately following the Root Directory
	uint32_t FirstDataSector = StartOfDataArea + RootDirCount;

	// Total number of sectors available in the data area (where file data is stored)
	uint32_t DataSec = TotSec - FirstDataSector;

	// The size of a single cluster in bytes (a cluster consists of one or more sectors)
	uint32_t ClusterSize = BOOTSECTOR.SectorsPerCluster * BOOTSECTOR.BytesPerSector;

	// The number of directory entries that can fit into one cluster (each entry is 32 bytes)
	uint32_t EntriesPerCluster = ClusterSize / 32;

	// Total number of clusters available in the data area
	uint32_t CountofClusters = DataSec / BOOTSECTOR.SectorsPerCluster;

	// Determine the FAT type based on the number of clusters
	if (CountofClusters < 4085)
	{
		FatData->FATType = FAT12;
	}
	else if (CountofClusters < 65525)
	{
		FatData->FATType = FAT16;
	}
	else
	{
		FatData->FATType = FAT32;
	}

	// Allocate memory for the private FAT32 data structure
	fatPrivData *priv = (fatPrivData *)malloc(sizeof(fatPrivData), FATInitDataPage);
	priv->ClusterSize = ClusterSize;

	// Root directory sector calculation and entries in root directory
	uint32_t RootDirSector;
	if (FatData->FATType != FAT32)
	{
		priv->RootDirSizeSec = EntriesPerCluster * BOOTSECTOR.SectorsPerCluster;
		RootDirSector = FirstDataSector + (BOOTSECTOR.EBR32.RootDirectoryCluster - 2) * BOOTSECTOR.SectorsPerCluster;
	}
	else
	{
		priv->RootDirSizeSec = RootDirCount;
		RootDirSector = StartOfDataArea;
		FirstDataSector = StartOfDataArea;
	}

	// Store key information in the FAT data structure
	FatData->FirstDataSector = FirstDataSector;
	FatData->CountofClusters = CountofClusters;
	FatData->FATSector = BOOTSECTOR.ReservedSectors;
	FatData->RootDirSector = RootDirSector;
	priv->BytesPerCluster = BOOTSECTOR.SectorsPerCluster * BOOTSECTOR.BytesPerSector;

	log_debug(MODULE, "Fat data: %s TotClusters %u FATsize %u", FatData->FATType == FAT32 ? "FAT32" : FatData->FATType == FAT16 ? "FAT16"
																																: "FAT12",
			  FatData->CountofClusters, FatData->FATSize);
	log_debug(MODULE, "FAT sector %u Root sector %u Data sector %u", FatData->FATSector, FatData->RootDirSector, FatData->FirstDataSector);
	log_debug(MODULE, "RootDirSizeSec %u ", priv->RootDirSizeSec);

	filesystemInfo_t *fs = (filesystemInfo_t *)malloc(sizeof(filesystemInfo_t), FATInitDataPage);
	if (FatData->FATType != FAT32)
	{
		fs->name = "FAT32";
	}
	else if (FatData->FATType != FAT16)
	{
		fs->name = "FAT16";
	}
	else if (FatData->FATType != FAT12)
	{
		fs->name = "FAT12";
	}

	fs->probe = (bool (*)(device_t *))FAT_Probe;
	fs->mount = (bool (*)(device_t *, void *))FAT_Mount;
	fs->read = (bool (*)(char *, uint8_t *, device_t *, void *))FAT_ReadFile;

	fs->priv_data = (void *)priv;

	dev->fs = fs;

	return true;
}

bool FAT_Mount(device_t *dev, void *privd)
{
	printf("Mounting fat32 on device %s (%d)\n", dev->name, dev->id);
	fatPrivData *priv = privd;
	if (FAT_ReadRootDirectory((char *)1, dev, priv))
		return 1;
	return 0;
}
