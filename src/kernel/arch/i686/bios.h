#pragma once

#include "defaultInclude.h"
#include "arch/i686/irq.h"

uint16_t ds();
uint16_t fs();
uint16_t gs();

typedef struct t_Registers16
{
union {
		struct {
			uint32_t edi;
			uint32_t esi;
			uint32_t ebp;
			uint32_t esp;
			uint32_t ebx;
			uint32_t edx;
			uint32_t ecx;
			uint32_t eax;
			uint32_t fsgs;
			uint32_t dses;
			uint32_t eflags;
		};
		struct {
			uint16_t di, hdi;
			uint16_t si, hsi;
			uint16_t bp, hbp;
			uint16_t sp, hsp;
			uint16_t bx, hbx;
			uint16_t dx, hdx;
			uint16_t cx, hcx;
			uint16_t ax, hax;
			uint16_t gs, fs;
			uint16_t es, ds;
			uint16_t flags, hflags;
		};
		struct {
			uint8_t dil, dih, edi2, edi3;
			uint8_t sil, sih, esi2, esi3;
			uint8_t bpl, bph, ebp2, ebp3;
			uint8_t spl, sph, esp2, esp3;
			uint8_t bl, bh, ebx2, ebx3;
			uint8_t dl, dh, edx2, edx3;
			uint8_t cl, ch, ecx2, ecx3;
			uint8_t al, ah, eax2, eax3;
		};
	};
} Registers16;

void BIOS_Init();
void CallInt(uint8_t intNum, Registers16 *regsInput, Registers16 *regsOutput);
void initregs(Registers16 *reg);
void print_reg(Registers * reg);
void print_reg16(Registers16 * reg);
