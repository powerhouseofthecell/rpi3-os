#ifndef RPI3OS_AARCH64_HH
#define RPI3OS_AARCH64_HH
#include "common/types.hh"

// Paged memory constants
#define PAGEOFFBITS     12                     // # bits in page offset
#define PAGEINDEXBITS   9                      // # bits in a page index level
#define PAGESIZE        (1UL << PAGEOFFBITS)   // Size of page in bytes
#define PAGEOFFMASK     (PAGESIZE - 1)

// Permission flags: define whether page is accessible
#define PTE_P           (1<<0)    // entry is Present
#define PTE_W           (1<<1)    // entry is Writeable
#define PTE_U           (1<<6)    // entry is User-accessible
#define PTE_PWU         (PTE_P | PTE_W | PTE_U)    // PTE_P | PTE_W | PTE_U

// Accessed flags: automatically turned on by processor
#define PTE_A           (1<<10)   // entry was Accessed (read/written)
#define PTE_D           0x40UL   // entry was Dirtied (written)

// Other special-purpose flags
#define PTE_PS          0x80UL   // entry has a large Page Size
#define PTE_PWT         0x8UL
#define PTE_PCD         0x10UL
#define PTE_XD          0x8000000000000000UL // entry is eXecute Disabled

// These flags are available for OS use (the processor ignores them):
#define PTE_OS1         0x200UL
#define PTE_OS2         0x400UL
#define PTE_OS3         0x800UL
// There are other flags too!

#define PTE_PAMASK      0x000FFFFFFFFFF000UL // physical address in non-PS entry
#define PTE_PS_PAMASK   0x000FFFFFFFFFE000UL // physical address in PS entry

#define VA_LOWMIN       0UL                  // min low canonical address
#define VA_LOWMAX       0x0000007fffffffffUL // max low canonical address
#define VA_LOWEND       0x0000008000000000UL // one past `VA_LOWMAX`
#define VA_HIGHMIN      0xFFFFFF8000000000UL // min high canonical address
#define VA_HIGHMAX      0xFFFFFFFFFFFFFFFFUL // max high canonical address
#define VA_NONCANONMAX  0x0000FFFFFFFFFFFFUL // max non-canonical address
#define VA_NONCANONEND  0x0001000000000000UL // one past `VA_NONCANONMAX`

#define PA_IOLOWMIN     0x00000000000A0000UL // min address of MMIO region 1
#define PA_IOLOWEND     0x0000000000100000UL // end address of MMIO region 1
#define PA_IOHIGHMIN    0x00000000C0000000UL // min address of MMIO region 2
#define PA_IOHIGHEND    0x0000000100000000UL // end address of MMIO region 2

// Parts of a paged address: page index, page offset
static inline int pageindex(uintptr_t addr, int level) {
    return (int) (addr >> (PAGEOFFBITS + level * PAGEINDEXBITS)) & 0x1FF;
}
static inline uintptr_t pageoffmask(int level) {
    return (1UL << (PAGEOFFBITS + level * PAGEINDEXBITS)) - 1;
}
static inline uintptr_t pageoffset(uintptr_t addr, int level) {
    return addr & pageoffmask(level);
}
static inline bool va_is_canonical(uintptr_t va) {
    return va <= VA_LOWMAX || va >= VA_HIGHMIN;
}

// a page type
typedef struct __attribute__((aligned(PAGESIZE))) page {
    uint8_t x[PAGESIZE];
} page;

// an entry within a pagetable
typedef uint64_t pageentry_t;
typedef struct __attribute__((aligned(PAGESIZE))) pagetable {
    pageentry_t entry[1 << PAGEINDEXBITS];
} pagetable;

#endif