#include "fat32.h"
#include "fatConfig.h"

#include "stdlib.h"
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

void GetName(char *shortName, char *output)
{
	char name[9] = {0}; // 8 chars + null terminator
	char ext[4] = {0};	// 3 chars + null terminator

	// Copy name and extension, trimming trailing spaces
	strncpy(name, shortName, 8);
	strncpy(ext, shortName + 8, 3);

	// Trim trailing spaces from name
	int i = 7;
	while (i >= 0 && name[i] == ' ')
	{
		name[i] = '\0';
		i--;
	}

	// Trim trailing spaces from ext
	i = 3;
	while (i >= 0 && ext[i] == ' ')
	{
		ext[i] = '\0';
		i--;
	}

	// Format result
	if (ext[0] != '\0')
	{
		sprintf(output, "%s.%s\0", name, ext);
	}
	else
	{
		sprintf(output, "%s\0", name);
	}
	int namelen = strlen(name);
	int extlen = strlen(ext);
	output[namelen + extlen + 1] = '\0';
}

void FAT_GetFATName(char *name, char *ret, bool needLFN)
{
	if (needLFN)
	{
		// Just copy the name as-is (for actual use, you'd need to encode it in LFN entries)
		strncpy(ret, name, 255); // LFN max length is 255 characters
		ret[255] = '\0';		 // Ensure null termination
		return;
	}
	getShortName(name, ret);
}

bool FAT_ReadSectors(void *buf, uint32_t sector, uint32_t length, device_t *dev, fatPrivData *priv)
{
#if debugFAT == 1
	log_info(MODULE, "FAT_ReadSectors(%p, %u, %u, %p, %p)", buf, sector, length, dev, priv);
	log_debug(MODULE, "file %s:%u", __FILE__, __LINE__);
#endif
	return dev->read(buf, sector, length, dev);
}
bool FAT_WriteSectors(void *buf, uint32_t sector, uint32_t length, device_t *dev, fatPrivData *priv)
{
#if debugFAT == 1
	log_debug(MODULE, "file %s:%u", __FILE__, __LINE__);
#endif
	return dev->write(buf, sector, length, dev);
}

uint32_t FAT_Read(void *buf, uint32_t sector, uint32_t byteCount, device_t *dev, fatPrivData *priv)
{
	uint8_t *result = (uint8_t *)buf;

	uint32_t bytesRead = 0;
	uint32_t count = 0;
	while (byteCount > bytesRead)
	{
		uint32_t leftToRead = min(SECTOR_SIZE, byteCount - bytesRead);
		uint32_t take = min(byteCount, leftToRead);
#if debugFAT == 1
		log_debug(MODULE, "bytesRead: %u - leftToRead: %u - take: %u - ", bytesRead, leftToRead, take);
#endif

		FAT_ReadSectors(result, sector + count, 1, dev, priv);
		result += take;
		bytesRead += take;
		count++;
	}

	return bytesRead;
}

bool FAT_ReadEntry(FAT_FileEntry *dirEntry, uint32_t sector, device_t *dev, fatPrivData *priv)
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

void FAT_readBootsector(device_t *dev, fatPrivData *priv)
{
	dev->read(FatData->BS.BootSectorBytes, 0, 1, dev);
}

void FAT_GetDir(FAT_DirectoryEntry *entry, FAT_Directory *dir, uint8_t *buffer, device_t *dev, fatPrivData *priv)
{
#if debugFAT == 1
	log_debug(MODULE, "file %s:%u", __FILE__, __LINE__);
#endif
	FAT_readBootsector(dev, priv);
	uint32_t firstSector = GETSECTORWC(entry->FirstClusterHigh, entry->FirstClusterLow);
#if debugFAT == 1
	log_info(MODULE, "FAT_ReadSectors(%p, %u, %u, %p, %p)", dir->entries, firstSector, BOOTSECTOR.SectorsPerCluster, dev, priv);
#endif
	FAT_ReadSectors((void *)dir->entries, firstSector, BOOTSECTOR.SectorsPerCluster, dev, priv);
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
#if debugFAT == 1
	log_info(MODULE, "BytesPerSector %u", BOOTSECTOR.BytesPerSector);
	log_info(MODULE, "fatIndex %u", fatIndex);
#endif
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

bool FAT_WriteClusters(uint8_t *buf, uint32_t firstCluster, uint32_t size, device_t *dev, fatPrivData *priv)
{
	uint32_t FatEOF = FAT_GetFatEOF();
	uint32_t cluster = firstCluster;
	uint32_t bytesRead = 0;
	while (cluster < FatEOF && bytesRead < size)
	{
		uint32_t sector = GETSECTOR(cluster);

		FAT_WriteSectors(buf + bytesRead, sector, BOOTSECTOR.SectorsPerCluster, dev, priv);

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
	// log_debug(MODULE, "file %s:%u", __FILE__, __LINE__);
	if (entryCount <= 0 || !lfnEntries || !output)
	{
		log_err("FAT32 LONG", "Exit error: entryCount: %u ", entryCount);
		*output = '\0';
		return;
	}

	// Sort LFN entries by Order (increasing)
	qsort(lfnEntries, entryCount, sizeof(FAT_LongFileEntry), FAT_CompareLFNBlocks);

	FAT_LFNBlock *LFNBlock = (FAT_LFNBlock *)malloc(sizeof(FAT_LFNBlock));
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
				free(LFNBlock);
				return; // End of name
			}

			int codepoint;
			utf16_to_codepoint((uint16_t *)&utf16_chars[j], &codepoint);
			outPtr = codepoint_to_utf8(codepoint, outPtr);
		}
	}

	*outPtr = '\0'; // Null-terminate the string
	free(LFNBlock);
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

void fatReadRootEntryCount(device_t *dev, fatPrivData *priv)
{
	if (FatData->FATType == FAT32)
	{
		priv->RootDirSizeSec = 2;
#if debugFAT == 1
		log_debug(MODULE, "fatReadRootEntryCount: FAT32 got %u", priv->RootDirSizeSec);
#endif
	}
	else
	{
		priv->RootDirSizeSec = ((BOOTSECTOR.DirEntryCount * DIR_ENTRY_SIZE) + (BOOTSECTOR.BytesPerSector - 1)) / BOOTSECTOR.BytesPerSector;
#if debugFAT == 1
		log_debug(MODULE, "fatReadRootEntryCount: FAT16/12 got %u", priv->RootDirSizeSec);
#endif
	}
}

bool FAT_ReadRootDirectory(char *filename, device_t *dev, fatPrivData *priv)
{
	fatReadRootEntryCount(dev, priv);

	// Calculate the first sector of the root directory
	uint32_t rootDirSector = FatData->RootDirSector;
	// log_debug(MODULE, "rootDirSector = %u", rootDirSector);

	// Read the first sector of the root directory (this contains the entries)
	// log_debug(MODULE, "root size = %u", priv->RootDirSizeSec);
	uint8_t *buffer = (uint8_t *)malloc(priv->RootDirSizeSec * (BOOTSECTOR.BytesPerSector * BOOTSECTOR.SectorsPerCluster));
	if (!buffer)
	{
		free(buffer);
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

	free(buffer);

	if (filename && (uint32_t)filename != 1)
		return false;
	return true;
}
bool FAT_ReadDirectory(char *filename, uint8_t *tempDir, device_t *dev, fatPrivData *priv)
{
#if debugFAT == 1
	log_debug(MODULE, "entered FAT_ReadDirectory(%s, %p, %p, %p)", filename, tempDir, dev, priv);
#endif
	FAT_Directory *dir = (FAT_Directory *)tempDir;
	if (!filename || !tempDir || !dev || !priv)
	{
		log_crit(MODULE, "Invalid arguments");
		return false;
	}

	if (filename[0] != '/')
	{
		log_crit(MODULE, "Only absolute paths are supported %s", filename);
		return false;
	}

	if (!filename)
	{
		log_crit(MODULE, "filename is null, Filepath is invalid");
		return false;
	}

	FAT_ReadRootDirectory((char *)1, dev, priv);
	if (FatData->RootDirectory.entryCount == 0)
	{
		log_crit(MODULE, "Root directory has no entries");
		return false;
	}

	char *segment = strtok(filename, "/");
	log_debug(MODULE, "segment = %s from %s", segment, filename);
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
#if debugFAT == 1
		log_debug(MODULE, "shortName = %s segment = %s", shortName, segment);

		log_debug(MODULE, "currentDir count %u", currentDir.entryCount);
#endif

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

		char *segment1 = strtok(NULL, "/");
		// If this is the last segment, we've found the file
		if (segment1 == NULL)
		{
			break;
		}
		segment = segment1;

		// Otherwise, move into the directory
		if (!(entry->Entry.Attributes & FAT_ATTRIBUTE_DIRECTORY))
		{
			log_crit(MODULE, "%s is not a directory", segment);
			return false;
		}
		uint8_t *buffer = (uint8_t *)malloc(priv->BytesPerCluster);
		FAT_GetDir(&entry->Entry, &currentDir, buffer, dev, priv);
		segment = strtok(NULL, "/");
		free(buffer);
	}
#if debugFAT == 1
	log_info(MODULE, "entry: name: %s, attr: %X, size: %u", entry->Entry.Name, entry->Entry.Attributes, entry->Entry.Size);
#endif
	// Read file data
	if ((entry->Entry.Attributes & FAT_ATTRIBUTE_DIRECTORY) == FAT_ATTRIBUTE_DIRECTORY)
	{
		log_info(MODULE, "Reading file %s", entry->Entry.Name);
		uint32_t firstCluster = GETCLUSTER(entry->Entry.FirstClusterHigh, entry->Entry.FirstClusterLow);
		log_info(MODULE, "FAT_ReadClusters(%p, %u, 1)", dir->entries, firstCluster);
		FAT_ReadClusters((uint8_t *)dir->entries, firstCluster, 1, dev, priv);
		int index = 0;
		dir->entryCount = 0;
		while (true)
		{
			log_info(MODULE, "entry: name: %s, attr: %X, size: %u", dir->entries[index].Entry.Name, dir->entries[index].Entry.Attributes, dir->entries[index].Entry.Size);
			if (dir->entries[index].Entry.Name[0] != '\0')
			{
				index++;
				dir->entryCount++;
				continue;
			}
			break;
		}

		return true;
	}
	log_crit(MODULE, "found nothing in FAT_ReadDirectory");
	return false;
}
bool FAT_ReadFile(char *fileName, uint8_t *buffer, device_t *dev, fatPrivData *priv)
{
#if debugFAT == 1
	log_debug(MODULE, "entered FAT_ReadFile(%s, %p, %p, %p)", fileName, buffer, dev, priv);
#endif
	if (!fileName || !buffer || !dev || !priv)
	{
		log_crit(MODULE, "Invalid arguments");
		return false;
	}

	if (fileName[0] != '/')
	{
		log_crit(MODULE, "Only absolute paths are supported %s", fileName);
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
	char *copyPath = malloc(MAX_PATH_SIZE);
	strcpy(copyPath, fileName);

	char *segment = strtok(fileName, "/");
	log_debug(MODULE, "segment = %s from %s", segment, fileName);
	if (segment == NULL)
	{
		strcpy(fileName, copyPath);
		free(copyPath);
		log_crit(MODULE, "Filepath is invalid");
		return false;
	}

	FAT_FileEntry *entry = NULL;
	FAT_Directory currentDir = FatData->RootDirectory;

	int i = 0;
	while (segment)
	{
		// init things
		log_debug(MODULE, "currentDir count %u", currentDir.entryCount);

		for (i = 0; i < currentDir.entryCount; i++)
		{
			char entryName[MAX_PATH_SIZE] = {'\0'};
			char segmentName[MAX_PATH_SIZE] = {'\0'};
			FAT_FileEntry *dirEntry = &currentDir.entries[i];
			log_debug(MODULE, "%u: %s 0x%X", i, dirEntry->Entry.Name, dirEntry->Entry.Attributes);
			if ((dirEntry->Entry.Attributes & FAT_ATTRIBUTE_LFN) == 0)
			{
				strncpy(entryName, (char *)dirEntry->Entry.Name, 11);
				FAT_GetFATName(segment, segmentName, false);
			}
			else
			{
				int LFNcount = dirEntry->Entry.Name[0] - 0x40;
				FAT_GetFATName(segment, segmentName, true);
				if (LFNcount == 1)
				{
					segmentName[0] = '\0';
					continue;
				}
				// log_debug(MODULE, "name: %s, %u, count %u", dirEntry->Entry.Name, dirEntry->Entry.Name[0], LFNcount);
				FAT_LongFileEntry LFNentries[LFNcount];
				for (size_t d = 0; d < LFNcount; d++)
				{
					LFNentries[d] = currentDir.entries[i].LongEntry;
					i++;
				}

				FAT_GetLfn(LFNentries, LFNcount, entryName);
			}

			if (entryName[0] != '\0' && segmentName[0] != '\0')
			{
#if debugFAT == 1
				fprintf(VFS_FD_DEBUG, "entryName = \"");
				for (size_t c = 0; c < 13; c++)
				{
					if (entryName[c + 1] == '\0')
					{
						fprintf(VFS_FD_DEBUG, "%X", entryName[c]);
						break;
					}
					fprintf(VFS_FD_DEBUG, "%X ", entryName[c]);
				}
				fprintf(VFS_FD_DEBUG, "\" segment = \"");
				for (size_t c = 0; c < 13; c++)
				{
					if (segmentName[c + 1] == '\0')
					{
						fprintf(VFS_FD_DEBUG, "%X", segmentName[c]);
						break;
					}
					fprintf(VFS_FD_DEBUG, "%X ", segmentName[c]);
				}
				fprintf(VFS_FD_DEBUG, "\"\n");

				log_debug(MODULE, "entryName = %s segment = %s", entryName, segmentName);
#endif
				if (strcmp(segmentName, entryName) == 0)
				{
					log_info(MODULE, "Found %s", segmentName);
					entry = dirEntry;
					break;
				}
			}
		}
		if (!entry)
		{
			strcpy(fileName, copyPath);
			free(copyPath);
			log_crit(MODULE, "File or directory not found: %s", segment);
			return false;
		}

		char *segment1 = strtok(NULL, "/");
		log_debug(MODULE, "segment1 = %s", segment1);
		// If this is the last segment, we've found the file
		if (segment1 == NULL)
		{
			break;
		}
		segment = segment1;

		// Otherwise, move into the directory
		if (!(entry->Entry.Attributes & FAT_ATTRIBUTE_DIRECTORY))
		{
			strcpy(fileName, copyPath);
			free(copyPath);
			log_crit(MODULE, "%s is not a directory", segment1);
			return false;
		}

		FAT_GetDir(&entry->Entry, &currentDir, NULL, dev, priv);
	}

	strcpy(fileName, copyPath);
	free(copyPath);
	// Read file data
	if ((entry->Entry.Attributes & FAT_ATTRIBUTE_DIRECTORY) == 0)
	{
		log_info(MODULE, "Reading file %s", entry->Entry.Name);
		uint32_t firstCluster = GETCLUSTER(entry->Entry.FirstClusterHigh, entry->Entry.FirstClusterLow);
		FAT_ReadClusters(buffer, firstCluster, entry->Entry.Size, dev, priv);
		return true;
	}
	return false;
}

bool FAT_WriteFile(char *fileName, uint8_t *buffer, device_t *dev, fatPrivData *priv)
{
	if (!fileName || !buffer || !dev || !priv)
	{
		log_crit(MODULE, "Invalid arguments");
		return false;
	}

	if (fileName[0] != '/')
	{
		log_crit(MODULE, "Only absolute paths are supported %s", fileName);
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

		char *segment1 = strtok(NULL, "/");
		// If this is the last segment, we've found the file
		if (segment1 == NULL)
		{
			break;
		}
		segment = segment1;

		// Otherwise, move into the directory
		if (!(entry->Entry.Attributes & FAT_ATTRIBUTE_DIRECTORY))
		{
			log_crit(MODULE, "%s is not a directory", segment);
			return false;
		}
		uint8_t *buffer = (uint8_t *)malloc(priv->BytesPerCluster);
		FAT_GetDir(&entry->Entry, &currentDir, buffer, dev, priv);
		segment = strtok(NULL, "/");
		free(buffer);
	}

	// Read file data
	if ((entry->Entry.Attributes & FAT_ATTRIBUTE_DIRECTORY) != FAT_ATTRIBUTE_DIRECTORY)
	{
		log_info(MODULE, "Writing file %s", entry->Entry.Name);
		uint32_t firstCluster = GETCLUSTER(entry->Entry.FirstClusterHigh, entry->Entry.FirstClusterLow);
		FAT_WriteClusters(buffer, firstCluster, entry->Entry.Size, dev, priv);
		return true;
	}
	return false;
}

bool FAT_FindEntry(char *path, void *ret, device_t *dev, fatPrivData *priv)
{
#if debugFAT == 1
	log_debug(MODULE, "entered FAT_FindEntry(%s, %p, %p)", path, dev, priv);
#endif
	if (!path || !dev || !priv)
	{
		log_crit(MODULE, "Invalid arguments");
		return false;
	}
	DirectoryEntry *dirRet = (DirectoryEntry *)ret;

	if (path[0] != '/')
	{
		log_crit(MODULE, "Only absolute paths are supported");
		return false;
	}

	if (!path)
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
	char *copyPath = malloc(MAX_PATH_SIZE);
	strcpy(copyPath, path);

	FAT_FileEntry *entry = NULL;
	FAT_Directory currentDir = FatData->RootDirectory;

	char *segment;

	// a count for the number of '/' in path
	int count = strcount((string)path, '/');
	segment = strtok(path, "/");
	if (segment == NULL)
	{
		strcpy(path, copyPath);
		free(copyPath);
		log_crit(MODULE, "Filepath is invalid");
		return false;
	}
	size_t i = 0;
	// while (segment != NULL)
	while (segment)
	{
		// init things

		for (i = 0; i < currentDir.entryCount; i++)
		{
			char entryName[MAX_PATH_SIZE] = {'\0'};
			char segmentName[MAX_PATH_SIZE] = {'\0'};
			FAT_FileEntry *dirEntry = &currentDir.entries[i];
			log_debug(MODULE, "%u: %s 0x%X", i, dirEntry->Entry.Name, dirEntry->Entry.Attributes);
			if ((dirEntry->Entry.Attributes & FAT_ATTRIBUTE_LFN) == 0)
			{
				strncpy(entryName, (char *)dirEntry->Entry.Name, 11);
				FAT_GetFATName(segment, segmentName, false);
			}
			else
			{
				int LFNcount = dirEntry->Entry.Name[0] - 0x40;
				FAT_GetFATName(segment, segmentName, true);
				if (LFNcount == 1)
				{
					segmentName[0] = '\0';
					continue;
				}
				// log_debug(MODULE, "name: %s, %u, count %u", dirEntry->Entry.Name, dirEntry->Entry.Name[0], LFNcount);
				FAT_LongFileEntry LFNentries[LFNcount];
				for (size_t d = 0; d < LFNcount; d++)
				{
					LFNentries[d] = currentDir.entries[i].LongEntry;
					i++;
				}

				FAT_GetLfn(LFNentries, LFNcount, entryName);
				// log_debug(MODULE, "output from FAT_GetLfn %s", entryName);
			}

			if (entryName[0] != '\0' && segmentName[0] != '\0')
			{
#if debugFAT == 1
				fprintf(VFS_FD_DEBUG, "entryName = \"");
				for (size_t c = 0; c < 13; c++)
				{
					if (entryName[c + 1] == '\0')
					{
						fprintf(VFS_FD_DEBUG, "%X", entryName[c]);
						break;
					}
					fprintf(VFS_FD_DEBUG, "%X ", entryName[c]);
				}
				fprintf(VFS_FD_DEBUG, "\" segment = \"");
				for (size_t c = 0; c < 13; c++)
				{
					if (segmentName[c + 1] == '\0')
					{
						fprintf(VFS_FD_DEBUG, "%X", segmentName[c]);
						break;
					}
					fprintf(VFS_FD_DEBUG, "%X ", segmentName[c]);
				}
				fprintf(VFS_FD_DEBUG, "\"\n");

				log_debug(MODULE, "entryName = %s segment = %s", entryName, segmentName);
#endif
				if (strcmp(segmentName, entryName) == 0)
				{
					// log_info(MODULE, "Found %s", entryName);
					entry = dirEntry;
					log_info(MODULE, "Found %s", segmentName);
					break;
				}
			}
		}
		if (!entry)
		{
			strcpy(path, copyPath);
			free(copyPath);
			log_crit(MODULE, "File or directory not found: %s", segment);
			return false;
		}

		char *segment1 = strtok(NULL, "/");
		log_debug(MODULE, "segment = %s", segment1);
		// If this is the last segment, we've found the file
		if (segment1 == NULL)
		{
			break;
		}
		segment = segment1;

		// Otherwise, move into the directory
		if (!(entry->Entry.Attributes & FAT_ATTRIBUTE_DIRECTORY))
		{
			strcpy(path, copyPath);
			free(copyPath);
			log_info(MODULE, "Found %s %x, %u", entry->Entry.Name, entry->Entry.Attributes, entry->Entry.Size);
			log_crit(MODULE, "%s is not a directory", segment1);
			return false;
		}

		uint8_t *buffer = (uint8_t *)malloc(priv->BytesPerCluster);
		FAT_GetDir(&entry->Entry, &currentDir, buffer, dev, priv);
		free(buffer);

		log_debug(MODULE, "segment = %s from %s", segment, path);
	}

	strcpy(path, copyPath);
	free(copyPath);
	if (!entry)
	{
		return false;
	}

	if ((entry->Entry.Attributes & FAT_ATTRIBUTE_LFN) == 0)
	{
		getShortName((char *)entry->Entry.Name, dirRet->name);
	}
	else
	{
		int LFNcount = entry->Entry.Name[0] - 0x40;
		FAT_LongFileEntry LFNentries[LFNcount];
		for (size_t d = 0; d < count; d++)
		{
			LFNentries[d] = currentDir.entries[i].LongEntry;
			i++;
		}

		FAT_GetLfn(LFNentries, LFNcount, dirRet->name);
	}

	dirRet->IsDirectory = (entry->Entry.Attributes & FAT_ATTRIBUTE_DIRECTORY) == FAT_ATTRIBUTE_DIRECTORY;
	dirRet->size = entry->Entry.Size;

	return true;
}

bool FAT_Probe(device_t *dev)
{
	log_info(MODULE, "Probing device %d", dev->id);
	if (!dev->read)
	{
		printf("Device has no read, skipped.\n");
		return 0;
	}

	FatData = (FAT_Data *)malloc(sizeof(FAT_Data));
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
	fatPrivData *priv = (fatPrivData *)malloc(sizeof(fatPrivData));
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

	filesystemInfo_t *fs = (filesystemInfo_t *)malloc(sizeof(filesystemInfo_t));
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
	fs->writefile = (bool (*)(char *, uint8_t *, uint32_t, device_t *, void *))FAT_WriteFile;
	fs->read_dir = (bool (*)(char *, uint8_t *, device_t *, void *))FAT_ReadDirectory;
	fs->find_entry = (bool (*)(char *, void *, device_t *, void *))FAT_FindEntry;

	fs->priv_data = (void *)priv;

	dev->fs = fs;

	return true;
}

bool FAT_Mount(device_t *dev, void *privd)
{
	printf("Mounting fat32 on device %s (%d)\n", dev->name, dev->id);
	fatPrivData *priv = privd;

	if (FAT_ReadRootDirectory((char *)1, dev, priv))
	{
		return 1;
	}
	return 0;
}

bool FAT_GetRoot(void *node, device_t *dev, void *privd)
{
	log_debug(MODULE, "node: %p, dev: %p, priv. %p", node, dev, privd);
	if (!dev || !privd)
	{
		log_crit(MODULE, "Invalid arguments for FAT_GetRoot");
		return false;
	}
	fatPrivData *priv = privd;

	vfs_node_t *vfsNode = (vfs_node_t *)node;

	vfsNode->permissions = VFS_DIR | VFS_READABLE | VFS_WRITABLE;
	vfsNode->size = 0;	// Size of the root directory is usually 0, as it contains entries for files and directories
	vfsNode->inode = 0; // Inode for root directory is usually 0

	// Set the name of the root directory
	strcpy(vfsNode->name, "/");

	return true;
}
