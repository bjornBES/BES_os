#include "elf.h"
#include "fat.h"
#include "memdefs.h"
#include "memory.h"
#include "minmax.h"
#include "stdio.h"

bool ELF_Read(Partition* part, const char* path, void** entryPoint)
{
    printf("Entering ELF_Read\r\n");
    printf("Loading ELF file %s\r\n", path);
    uint8_t* headerBuffer = MEMORY_ELF_ADDR;
    uint8_t* loadBuffer = MEMORY_LOAD_KERNEL;
    uint32_t filePos = 0;
    uint32_t read;
    // Read header
    FAT_File* fd = FAT_Open(part, path);
    printf("FAT_Open returned %p\r\n", fd);
    if ((read = FAT_Read(part, fd, sizeof(ELFHeader), headerBuffer)) != sizeof(ELFHeader))
    {
        printf("ELF Load error!\n");
        return false;
    }
    filePos += read;

    // validate header
    bool ok = true;
    ELFHeader* header = (ELFHeader*)headerBuffer;
    printf("Header Magic: ");
    for (uint8_t i = 0; i < 4; i++)
    {
        printf("%X", header->Magic[i]);
    }
    printf("\r\n");
    
    printf("true? %s\r\n", memcmp(header->Magic, ELF_MAGIC, 4) != 0 ? "true" : "false");
    printf("true? %s\r\n", ok = ok ? "true" : "false");

    ok = ok && (memcmp(header->Magic, ELF_MAGIC, 4) != 0);
    printf("1 OK: %d\r\n", ok);
    ok = ok && (header->Bitness == ELF_BITNESS_32BIT);
    printf("2 OK: %d\r\n", ok);
    ok = ok && (header->Endianness == ELF_ENDIANNESS_LITTLE);
    printf("3 OK: %d\r\n", ok);
    ok = ok && (header->ELFHeaderVersion == 1);
    printf("4 OK: %d\r\n", ok);
    ok = ok && (header->ELFVersion == 1);
    printf("5 OK: %d\r\n", ok);
    ok = ok && (header->Type == ELF_TYPE_EXECUTABLE);
    printf("6 OK: %d\r\n", ok);
    ok = ok && (header->InstructionSet == ELF_INSTRUCTION_SET_X86);
    printf("7 OK: %d\r\n", ok);
    printf("Header: Magic: %s, Bitness: %d, Endianness: %d, Version: %d, Type: %d, InstructionSet: %d\r\n",
        header->Magic, header->Bitness, header->Endianness, header->ELFHeaderVersion,
        header->Type, header->InstructionSet);
    printf("Header: ProgramEntryPosition: 0x%X, ProgramHeaderTablePosition: %d, SectionHeaderTablePosition: 0x%X\r\n",
        header->ProgramEntryPosition, header->ProgramHeaderTablePosition, header->SectionHeaderTablePosition);
    printf("Header: Flags: %d, HeaderSize: %d, ProgramHeaderTableEntrySize: %d, ProgramHeaderTableEntryCount: %d\r\n",
        header->Flags, header->HeaderSize, header->ProgramHeaderTableEntrySize, header->ProgramHeaderTableEntryCount);
    printf("Header: SectionHeaderTableEntrySize: %d, SectionHeaderTableEntryCount: %d, SectionNamesIndex: %d\r\n",
        header->SectionHeaderTableEntrySize, header->SectionHeaderTableEntryCount, header->SectionNamesIndex);

    *entryPoint = (void*)header->ProgramEntryPosition;

    // load program header
    uint32_t programHeaderOffset = header->ProgramHeaderTablePosition;
    uint32_t programHeaderSize = header->ProgramHeaderTableEntrySize * header->ProgramHeaderTableEntryCount;
    uint32_t programHeaderTableEntrySize = header->ProgramHeaderTableEntrySize;
    uint32_t programHeaderTableEntryCount = header->ProgramHeaderTableEntryCount;

    filePos += FAT_Read(part, fd, programHeaderOffset - filePos, headerBuffer);
    if ((read = FAT_Read(part, fd, programHeaderSize, headerBuffer)) != programHeaderSize)
    {
        printf("ELF Load error!\n");
        return false;
    }
    filePos += read;
    FAT_Close(fd);

    // parse program header entries
    for (uint32_t i = 0; i < programHeaderTableEntryCount; i++)
    {
        ELFProgramHeader* progHeader = (ELFProgramHeader*)(headerBuffer + i * programHeaderTableEntrySize);
        printf("Program header %d: Type: %d, Offset: %d, VirtualAddress: %x, PhysicalAddress: %x, FileSize: %d, MemorySize: %d, Flags: %x, Align: %d\n",
            i, progHeader->Type, progHeader->Offset, progHeader->VirtualAddress, progHeader->PhysicalAddress,
            progHeader->FileSize, progHeader->MemorySize, progHeader->Flags, progHeader->Align);
        if (progHeader->Type == ELF_PROGRAM_TYPE_LOAD) {
            // TODO: validate that the program doesn't overwrite the stage2
            uint8_t* virtAddress = (uint8_t*)progHeader->VirtualAddress;
            memset(virtAddress, 0, progHeader->MemorySize);
            
            // ugly nasty seeking
            // TODO: proper seeking
            fd = FAT_Open(part, path);
            while (progHeader->Offset > 0) 
            {
                uint32_t shouldRead = min(progHeader->Offset, MEMORY_LOAD_SIZE);
                printf("Bytes to read: %d\n", shouldRead);
                read = FAT_Read(part, fd, shouldRead, loadBuffer);
                printf("Bytes read: %d\n", read);
                if (read != shouldRead) {
                    printf("ELF Load error!\n");
                    return false;
                }
                progHeader->Offset -= read;
            }
            
            // read program
            while (progHeader->FileSize > 0) 
            {
                uint32_t shouldRead = min(progHeader->FileSize, MEMORY_LOAD_SIZE);
                printf("Bytes to read: %d\n", shouldRead);
                read = FAT_Read(part, fd, shouldRead, loadBuffer);
                printf("Bytes read: %d\n", read);
                if (read != shouldRead) {
                    printf("ELF Load error!\n");
                    return false;
                }
                progHeader->FileSize -= read;

                memcpy(virtAddress, loadBuffer, read);
                virtAddress += read;
            }

            FAT_Close(fd);
        }
    }

    return true;
}