#ifndef _MMU_H
#define _MMU_H
#include "kernel/gpio.hh"

// ***************************************
// SCTLR_EL1, System Control Register (EL1), Page 2025 of AArch64-Reference-Manual.
// ***************************************

#define SCTLR_RESERVED                  (3 << 28) | (3 << 22) | (1 << 20) | (1 << 11)
#define SCTLR_EE_LITTLE_ENDIAN          (0 << 25)
#define SCTLR_EOE_LITTLE_ENDIAN         (0 << 24)
#define SCTLR_I_CACHE_DISABLED          (0 << 12)
#define SCTLR_D_CACHE_DISABLED          (0 << 2)
#define SCTLR_MMU_DISABLED              (0 << 0)
#define SCTLR_MMU_ENABLED               (1 << 0)

#define SCTLR_VALUE_MMU_DISABLED	(SCTLR_RESERVED | SCTLR_EE_LITTLE_ENDIAN | SCTLR_I_CACHE_DISABLED | SCTLR_D_CACHE_DISABLED | SCTLR_MMU_DISABLED)

// ***************************************
// HCR_EL2, Hypervisor Configuration Register (EL2), Page 1923 of AArch64-Reference-Manual.
// ***************************************

#define HCR_RW	    (1 << 31)
#define HCR_VALUE   HCR_RW

// ***************************************
// SCR_EL3, Secure Configuration Register (EL3), Page 2022 of AArch64-Reference-Manual.
// ***************************************

#define SCR_RESERVED		(3 << 4)
#define SCR_RW				(1 << 10)
#define SCR_NS				(1 << 0)
#define SCR_VALUE	    	(SCR_RESERVED | SCR_RW | SCR_NS)

// ***************************************
// SPSR_EL3, Saved Program Status Register (EL3) Page 288 of AArch64-Reference-Manual.
// ***************************************

#define SPSR_MASK_ALL 		(7 << 6)
#define SPSR_EL1h			(5 << 0)
#define SPSR_VALUE			(SPSR_MASK_ALL | SPSR_EL1h)

// ***************************************
// ESR_EL1, Exception Syndrome Register (EL1). Page 1899 of AArch64-Reference-Manual.
// ***************************************

#define ESR_ELx_EC_SHIFT		26
#define ESR_ELx_EC_SVC64		0x15
#define ESR_ELx_EC_DABT_LOW		0x24
// end sysregs

#define MM_TYPE_PAGE_TABLE		0x3
#define MM_TYPE_PAGE 			0x3
#define MM_TYPE_BLOCK			0x1
#define MM_ACCESS			(0x1 << 10)
#define MM_ACCESS_PERMISSION		(0x01 << 6) 

/*
 * Memory region attributes:
 *
 *   n = AttrIndx[2:0]
 *			n	MAIR
 *   DEVICE_nGnRnE	000	00000000
 *   NORMAL_NC		001	01000100
 */
#define MT_DEVICE_nGnRnE 	    0x0
#define MT_NORMAL_NC			0x1
#define MT_DEVICE_nGnRnE_FLAGS  0x00
#define MT_NORMAL_NC_FLAGS  	0x44
#define MAIR_VALUE			    (MT_DEVICE_nGnRnE_FLAGS << (8 * MT_DEVICE_nGnRnE)) | (MT_NORMAL_NC_FLAGS << (8 * MT_NORMAL_NC))

#define MMU_FLAGS	 		(MM_TYPE_BLOCK | (MT_NORMAL_NC << 2) | MM_ACCESS)	
#define MMU_DEVICE_FLAGS    (MM_TYPE_BLOCK | (MT_DEVICE_nGnRnE << 2) | MM_ACCESS)	
#define MMU_PTE_FLAGS	    (MM_TYPE_PAGE | (MT_NORMAL_NC << 2) | MM_ACCESS | MM_ACCESS_PERMISSION)	

#define TCR_T0SZ		(64 - 48) 
#define TCR_T1SZ		((64 - 48) << 16)
#define TCR_TG0_4K	    (0 << 14)
#define TCR_TG1_4K		(2 << 30)
#define TCR_VALUE	    (TCR_T0SZ | TCR_T1SZ | TCR_TG0_4K | TCR_TG1_4K)

#define VA_START 			0xffff000000000000

#define PHYS_MEMORY_SIZE 	0x40000000	

#define PAGE_MASK			0xfffffffffffff000
#define PAGE_SHIFT	 		12
#define TABLE_SHIFT 		9
#define SECTION_SHIFT		(PAGE_SHIFT + TABLE_SHIFT)

#define PAGE_SIZE   		(1 << PAGE_SHIFT)	
#define SECTION_SIZE		(1 << SECTION_SHIFT)	

#define LOW_MEMORY          (2 * SECTION_SIZE)
#define HIGH_MEMORY         MMIO_BASE

#define PAGING_MEMORY 		(HIGH_MEMORY - LOW_MEMORY)
#define PAGING_PAGES 		(PAGING_MEMORY/PAGE_SIZE)

#define PTRS_PER_TABLE		(1 << TABLE_SHIFT)

#define PGD_SHIFT			PAGE_SHIFT + 3*TABLE_SHIFT
#define PUD_SHIFT			PAGE_SHIFT + 2*TABLE_SHIFT
#define PMD_SHIFT			PAGE_SHIFT + TABLE_SHIFT

#define PG_DIR_SIZE			(3 * PAGE_SIZE)

// initialize the memory management unit for ARM64
void mmu_init();

#endif