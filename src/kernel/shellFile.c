#include "shellFile.h"

#include <libcob.h>

#include "debug.h"
#include "memory.h"
#include "stdio.h"
#include "string.h"
#include "CobolCalls.h"
#include "task/process.h"

#include "arch/i686/gdt.h"
#include "arch/i686/idt.h"
#include "arch/i686/bios.h"

#include "drivers/VGA/vga.h"
#include "drivers/Keyboard/keyboard.h"
#include "drivers/PS2/8042_controller.h"

#include "syscall/systemcall.h"

#include "hal/vfs.h"

#include "printfDriver/printf.h"
#include "unistd.h"
#include "ctype.h"

#define MODULE "SHELL"
extern uint8_t user_stack_top;

void ReadLine(char *buffer)
{
    uint8_t bufferIndex = 0;
    while (true)
    {
        char c = KeyboardGetKey();
        if (c == 0)
        {
            continue;
        }
        if (c == '\n')
        {
            return;
        }
        if (c == '\b')
        {
            if (bufferIndex > 0)
            {
                int x = 0;
                int y = 0;
                VGA_getcursor(&x, &y);
                log_debug("MAIN", "backspace %u (X,Y): (%u, %u)", bufferIndex, x, y);
                bufferIndex--;
                x--;
                VGA_setcursor(x, y);
                printf(" ");
                buffer[bufferIndex] = 0;
                VGA_setcursor(x, y);
            }
        }
        else
        {
            buffer[bufferIndex] = c;
            bufferIndex++;
            printf("%c", c);
            // log_debug("MAIN", "char: %c - %u 0x%X", c, (uint8_t)c, (uint8_t)c);
        }
    }
}

bool cmpCommand(char *command, char *buffer)
{
    while (*command && *buffer)
    {
        if (!(*command == *buffer || toupper(*command) == *buffer))
        {
            return false;
        }
        command++;
        buffer++;
    }
    return true;
}

extern char __userProg_start[];
extern void setSS(uint32_t ss);
extern uint32_t kernelStack;

void EnterShell()
{
    char *inputBuffer = (char *)malloc(256);
    char *command = (char *)malloc(64);
    char *cmdPath = (char *)malloc(MAX_PATH_SIZE);
    strcpy(cmdPath, "/ata0");
    uint32_t bufferSize = 4096;
    void *buffer = malloc(4096);
    VGA_clrscr();
    printf("BES OS v0.1\n");
    printf("This operating system is under construction.\n\n");
    while (true)
    {
        printf("%s$", cmdPath);
        memset(inputBuffer, 0, 256);
        ReadLine(inputBuffer);

        int count = strcount(inputBuffer, ' ');
        char *loc = strtok(inputBuffer, " ");
        char *argv[count + 1];

        putc('\n');

        log_debug("MAIN", "loc = %s, count = %u", loc, count);
        int argc = 0;

        for (size_t i = 0; i < count + 1; i++)
        {
            if (loc == NULL)
            {
                log_debug("MAIN", "continue loc = %s", loc);
                continue;
            }
            argc++;
            argv[i] = loc;
            log_debug("MAIN", "%u: argv[i] = %s", i, argv[i]);
            if (i == 0)
            {
                command = memcpy(command, loc, sizeof(loc) + 1);
            }
            loc = strtok(NULL, " ");
        }

        if (argv[0][0] == '\0')
        {
            continue;
        }

        log_debug("MAIN", "count = %u", count);

        // Command layout
        // device/bin/Command.cod [args]

        if (cmpCommand("cmd", argv[0]) == true)
        {

            if (cmpCommand("clear", argv[1]) == true)
            {
                VGA_clrscr();
            }
            if (cmpCommand("ex", argv[1]) == true)
            {
                if (count < 2)
                {
                    printf("Usage: int <address> <fd>\n");
                    continue;
                }

                fd_t fd = VFS_FD_STDOUT;
                uint32_t address = 0;
                atoi(argv[2], (int *)&address);
                atoi(argv[3], &fd);

                uint8_t *u8Buffer = (uint8_t *)address;
                for (uint16_t i = 0; i < bufferSize; i++)
                {
                    uint8_t word = u8Buffer[i];

                    if (i % 16 == 0)
                    {
                        fprintf(fd, "\n%04X\t", i);
                    }
                    fprintf(fd, "%02X,", word);
                }
            }
            if (cmpCommand("mem-status", argv[1]) == true)
            {
                mmPrintStatus();
            }
            if (cmpCommand("call", argv[1]) == true)
            {
                cob_init(count + 1, argv);
                MainKernelInCobol();
            }
            if (cmpCommand("int", argv[1]) == true)
            {
                if (count < 1)
                {
                    printf("Usage: int <interrupt>\n");
                }
                int interrupt = 0;
                atoi(argv[2], &interrupt);
                IntRegisters regs;
                initregs(&regs);
                regs.eax = 0x55AA55AA; // just a test value
                regs.ebx = 0x12345678; // just a test value
                CallInt(interrupt, &regs);
            }
            if (cmpCommand("c-d", argv[1]) == true)
            {
                if (count < 1)
                {
                    printf("Usage: change-device <device>\n");
                }
                char *path = argv[2];
                if (path[0] != '/')
                {
                    printf("Path must be absolute!\n");
                    continue;
                }
                strcpy(cmdPath, path);
                log_debug("MAIN", "cmdPath = %s", cmdPath);
                continue;
            }
            if (cmpCommand("read-file", argv[1]) == true)
            {
                if (count < 1)
                {
                    printf("Usage: read <file>\n");
                }
                char *name = argv[2];
                char *path = (char *)malloc(MAX_PATH_SIZE);
                strcat(path, cmdPath);
                strcat(path, "/");
                strcat(path, name);
                fd_t file = VFS_Open(path);
                VFS_Read(file, buffer, bufferSize);
                VFS_Close(file);
                free(path);
                continue;
            }
            if (cmpCommand("read", argv[1]) == true)
            {
                if (count < 2)
                {
                    printf("Usage: read <lbs> <count>\n");
                }
                uint32_t lbs = 0;
                uint32_t sectorcount = 0;
                atoi(argv[2], (int *)&lbs);
                atoi(argv[3], (int *)&sectorcount);
                if (bufferSize < sectorcount * 512)
                {
                    log_debug("MAIN", "reallocating buffer %u", sectorcount * 512);
                    buffer = realloc(buffer, sectorcount * 512);
                    bufferSize = sectorcount * 512;
                    memset(buffer, 0, bufferSize);
                }
                ATA_read(buffer, lbs, sectorcount, GetDeviceUsingId(32));
            }
            if (cmpCommand("hex", argv[1]) == true)
            {
                if (count < 1)
                {
                    printf("Usage: hex <file dis>\n");
                }
                fd_t fd = VFS_FD_STDOUT;
                atoi(argv[2], &fd);
                uint8_t *u8Buffer = (uint8_t *)buffer;
                for (uint16_t i = 0; i < bufferSize; i++)
                {
                    uint8_t word = u8Buffer[i];

                    if (i % 16 == 0)
                    {
                        fprintf(fd, "\n%04X\t", i);
                    }
                    fprintf(fd, "%02X,", word);
                }
            }
            if (cmpCommand("info", argv[1]) == true)
            {
                if (count < 1)
                {
                    printf("Usage: hex <file dis>\n");
                }
                fd_t fd = VFS_FD_STDOUT;
                atoi(argv[2], &fd);
                fprintf(fd, "\nBufferSize = %u\n", bufferSize);
            }
            if (cmpCommand("mount", argv[1]) == true)
            {
                if (count < 2)
                {
                    printf("Usage: mount <device> <loc>\n");
                }
                int device = 0;
                atoi(argv[2], &device);
                char *loc = argv[3];

                device_t *dev = GetDeviceUsingId(device);
                if (dev == NULL)
                {
                    printf("Device %u not found!\n", device);
                    continue;
                }

                if (MountDevice(dev, loc))
                {
                    printf("Mounted %s on %s\n", dev->name, loc);
                    log_info("MAIN", "Mounted %s on %s", dev->name, loc);
                }
                else
                {
                    printf("Failed to mount %s on %s\n", dev->name, loc);
                    log_err("MAIN", "Failed to mount %s on %s", dev->name, loc);
                }
            }
        }
        else
        {
            char *path = (char *)malloc(MAX_PATH_SIZE);
            char *binpath = (char *)malloc(MAX_PATH_SIZE);

            char *fileName = (char *)malloc(MAX_PATH_SIZE);
            fileName = strtok(argv[0], "/");
            log_debug(MODULE, "CHECK: fileName = %s, path = %s", fileName, path);
            if (strcount(fileName, '.') == 0)
            {
                strcat(fileName, ".bin");
            }
            log_debug(MODULE, "CHECK: fileName = %s, path = %s", fileName, path);

            strtok(NULL, "/");

            sprintf(path, "%s/bin/%s", cmdPath, fileName);
            sprintf(binpath, "%s/bin", cmdPath);
            fd_t binDirFD = open(binpath, 0);
            DirectoryEntries binDir;
            binDir.entries = calloc(sizeof(DirectoryEntry), 12);
            VFS_Readdir(binDirFD, &binDir);
            uint32_t bytesRead;
            for (size_t i = 0; i < binDir.entryCount; i++)
            {
                log_debug(MODULE, "filename %s == bindir %s", fileName, binDir.entries[i].name);
                if (!strcasecmp(fileName, binDir.entries[i].name))
                {
                    log_debug(MODULE, "path: %s", path);
                    fd_t file = VFS_Open(path);
                    int size = VFS_GetSize(file);
                    size = ROUNDUP(size, 512);
                    log_debug(MODULE, "size = %u", size);
                    bytesRead = 0;

                    uint32_t sector = size / 512;
                    buffer = realloc(buffer, (sector * 2) * 512);
                    bufferSize = (sector * 2) * 512;

                    while (bytesRead < size)
                    {
                        bytesRead += VFS_Read(file, buffer, 512);
                        lseek(file, (sector * 2) * 512, SEEK_CUR);
                    }
                    VFS_Close(file);
                }
            }

            VFS_Close(binDirFD);

            if (cmpCommand("CEXE", buffer))
            {
                log_debug(MODULE, "using the cexe header at %p", buffer);
                CexeProgramHeader_t *cexeHeader = (CexeProgramHeader_t *)buffer;

                // Header fields
                fprintf(VFS_FD_DEBUG, "== Cexe Header ==\n");
                fprintf(VFS_FD_DEBUG, "Magic: %4s\n", cexeHeader->header.Header.magic);
                fprintf(VFS_FD_DEBUG, "Version: %u\n", cexeHeader->header.Header.version);
                fprintf(VFS_FD_DEBUG, "Program Length: %u\n", cexeHeader->header.Header.programLength);
                fprintf(VFS_FD_DEBUG, "Physical Offset: %u\n", cexeHeader->header.Header.physOffset);
                fprintf(VFS_FD_DEBUG, "\n");

                // Section entries
                for (int i = 0; i < 4; ++i)
                {
                    CexeSectionEntry_t entry = cexeHeader->entries[i];
                    fprintf(VFS_FD_DEBUG, "== Section Entry %d ==\n", i);
                    fprintf(VFS_FD_DEBUG, "Ident: %2s\n", entry.section.ident);
                    fprintf(VFS_FD_DEBUG, "Flags: 0x%02X\n", entry.section.flags);
                    fprintf(VFS_FD_DEBUG, "Offset: 0x%08x%08x\n", entry.section.offset.u32Offset[1], entry.section.offset.u32Offset[0]);
                    fprintf(VFS_FD_DEBUG, "Size: %u\n", entry.section.size);
                }

                uint32_t programSize = cexeHeader->header.Header.programLength;
                CexeSectionEntry_t *textSection = &cexeHeader->entries[0];
                uint8_t *code = buffer + textSection->section.offset.u32Offset[0] + 0x60;
                log_info(MODULE, "programSize = %u, code = 0x%p", programSize, code);
                memcpy(__userProg_start, code, programSize);
            }
            else
            {
                log_debug(MODULE, "flat binary");
                memcpy(__userProg_start, buffer, bytesRead);
            }
            usermodeFunc = (uint32_t)__userProg_start;
            __asm__("pusha");
            __asm__("mov %%esp, %0" : "=r"(kernelStack));
            process_t* process = makeProcess(usermodeFunc, (uint32_t)(&user_stack_top));
            log_debug(MODULE, "bytesRead = %u, usermodeFunc = 0x%p", bytesRead, usermodeFunc);
            printf("Starting usermode program at 0x%p\n", (void *)usermodeFunc);
            runProcess(process);
            // __asm__("mov %%esi, %0" : : "r"(kernelStack));
            setSS(i686_GDT_DATA_SEGMENT);
            log_debug(MODULE, "return back from user");
            ASM_INT2();

            IDT_Load();

            ps2_probe();

            fprintf(VFS_FD_DEBUG, "Back in kernel mode\n");
            uint32_t data;
            __asm__("mov %%gs:0x0100000, %0" : "=r"(data));
            fprintf(VFS_FD_DEBUG, "at [0x0100000] = %x\n", data);
            __asm__("mov %%gs:0x0, %0" : "=r"(data));
            fprintf(VFS_FD_DEBUG, "at [0] = %x\n", data);


            continue;
        }
    }

    free(command);
    free(buffer);
}