#ifndef PROC_HH
#define PROC_HH

#include "common/types.hh"
#include "kernel/pagetable.hh"
#include "kernel/regstate.hh"

// Process state constants
#define P_FREE      0                   // free slot
#define P_RUNNABLE  1                   // runnable process
#define P_BLOCKED   2                   // blocked process
#define P_BROKEN    3                   // faulted process

// Process descriptor type
struct proc {
    pagetable* pt;                      // process's page table
    pid_t pid;                          // process ID
    int state = P_FREE;                          // process state (see above)
    regstate regs;                      // process's registers
};

// Process table
// maximum number of processes
#define NPROC 16
extern proc ptable[NPROC];

#endif