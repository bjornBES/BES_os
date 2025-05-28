#include "elf.h"
#include "fat.h"
#include "memdefs.h"
#include "memory.h"
#include "minmax.h"
#include "stdio.h"

bool ELF_Read(Partition *part, const char *path, void **entryPoint)
{
    uint8_t *headerBuffer = MEMORY_ELF_ADDR;
    uint8_t *loadBuffer = MEMORY_LOAD_KERNEL;
    uint32_t filePos = 0;
    uint32_t read;
    // Read header
    FAT_File *fd = FAT_Open(part, path);
    if ((read = FAT_Read(part, fd, sizeof(ELFHeader), headerBuffer)) != sizeof(ELFHeader))
    {
        printf("ELF Load error!\n");
        return false;
    }
    filePos += read;

    // validate header
    bool ok = true;
    ELFHeader *header = (ELFHeader *)headerBuffer;

    ok = ok && (memcmp(header->Magic, ELF_MAGIC, 4) != 0);
    ok = ok && (header->Bitness == ELF_BITNESS_32BIT);
    ok = ok && (header->Endianness == ELF_ENDIANNESS_LITTLE);
    ok = ok && (header->ELFHeaderVersion == 1);
    ok = ok && (header->ELFVersion == 1);
    ok = ok && (header->Type == ELF_TYPE_EXECUTABLE);
    ok = ok && (header->InstructionSet == ELF_INSTRUCTION_SET_X86);

    // Validate programHeaderOffset
    if (header->ProgramHeaderTablePosition < sizeof(ELFHeader)) {
        printf("Error: Invalid programHeaderOffset (%d). It must be >= ELF header size (%lu).\n",
               header->ProgramHeaderTablePosition, sizeof(ELFHeader));
        FAT_Close(fd);
        return false;
    }

    *entryPoint = (void *)header->ProgramEntryPosition;

    // load program header
    uint32_t programHeaderOffset = header->ProgramHeaderTablePosition;
    uint32_t programHeaderSize = header->ProgramHeaderTableEntrySize * header->ProgramHeaderTableEntryCount;
    uint32_t programHeaderTableEntrySize = header->ProgramHeaderTableEntrySize;
    uint32_t programHeaderTableEntryCount = header->ProgramHeaderTableEntryCount;

    // Calculate alignment bytes
    int32_t alignBytes = programHeaderOffset - filePos;
    if (alignBytes < 0) {
        printf("Error: filePos (%d) is already beyond programHeaderOffset (%d)\n", filePos, programHeaderOffset);
        FAT_Close(fd);
        return false;
    }
    // Read alignment bytes
    filePos += FAT_Read(part, fd, alignBytes, headerBuffer);

    if ((read = FAT_Read(part, fd, programHeaderSize, headerBuffer)) != programHeaderSize) {
        printf("ELF Load error! Expected %d bytes, but read %d bytes.\n", programHeaderSize, read);
        FAT_Close(fd);
        return false;
    }
    filePos += read;

    // Parse program header entries
    for (uint32_t i = 0; i < programHeaderTableEntryCount; i++) {
        ELFProgramHeader *progHeader = (ELFProgramHeader *)(headerBuffer + i * programHeaderTableEntrySize);

        if (progHeader->Type == ELF_PROGRAM_TYPE_LOAD) {
            // Validate offsets and sizes
            if (progHeader->Offset < filePos) {
                printf("Invalid program header: Offset (%d) is less than current filePos (%d)\n", progHeader->Offset, filePos);
                FAT_Close(fd);
                return false;
            }

            // Validate that the program header's data fits within the kernel file
            if (progHeader->FileSize - progHeader->Offset > progHeader->FileSize) {
                printf("Invalid program header: Offset + FileSize (%d) exceeds kernel file size (%d)\n",
                    progHeader->FileSize - progHeader->Offset, progHeader->FileSize);
                FAT_Close(fd);
                return false;
            }

            uint8_t *virtAddress = (uint8_t *)progHeader->VirtualAddress;
            memset(virtAddress, 0, progHeader->MemorySize);

            loadBuffer = virtAddress;

            // Seek to the program header offset
            if (FAT_Read(part, fd, progHeader->Offset - filePos, loadBuffer) != progHeader->Offset - filePos) {
                printf("Failed to seek to program header offset\n");
                FAT_Close(fd);
                return false;
            }
            // filePos = 0;
            filePos = progHeader->Offset;

            // Read program data
            while (progHeader->FileSize > 0) {
                uint32_t shouldRead = min(progHeader->FileSize, MEMORY_LOAD_SIZE);
                read = FAT_Read(part, fd, shouldRead, loadBuffer);
                if (read != shouldRead) {
                    printf("Failed to read program data\n");
                    FAT_Close(fd);
                    return false;
                }
                /*
                for (uint16_t i = 0; i < 512; i++)
                {
                    uint8_t word = loadBuffer[(i + 0xA00)];
                    if (i % 16 == 0)
                    {
                        printf("\n%X\t", i + 0xA00);
                    }
                    printf("%X,", word);
                }
                */
                progHeader->FileSize -= read;

                // memcpy(virtAddress, loadBuffer, read);
                virtAddress += read;
                loadBuffer += read;
            }
        }
    }
    FAT_Close(fd);

    return true;
}