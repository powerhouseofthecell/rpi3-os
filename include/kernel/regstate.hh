#ifndef REGSTATE_HH
#define REGSTATE_HH

#include "common/types.hh"

typedef struct regstate {
    // special system registers
    uint64_t reg_spsr;
    uint64_t reg_elr;
    uint64_t reg_tpidr;
    uint64_t reg_sp;

    // general purpose registers
    uint64_t reg_x30;
    uint64_t reg_xzr;

    uint64_t reg_x28;
    uint64_t reg_x29;

    uint64_t reg_x26;
    uint64_t reg_x27;

    uint64_t reg_x24;
    uint64_t reg_x25;

    uint64_t reg_x22;
    uint64_t reg_x23;

    uint64_t reg_x20;
    uint64_t reg_x21;

    uint64_t reg_x18;
    uint64_t reg_x19;

    uint64_t reg_x16;
    uint64_t reg_x17;

    uint64_t reg_x14;
    uint64_t reg_x15;

    uint64_t reg_x12;
    uint64_t reg_x13;

    uint64_t reg_x10;
    uint64_t reg_x11;

    uint64_t reg_x8;
    uint64_t reg_x9;

    uint64_t reg_x6;
    uint64_t reg_x7;

    uint64_t reg_x4;
    uint64_t reg_x5;

    uint64_t reg_x2;
    uint64_t reg_x3;

    uint64_t reg_x0;
    uint64_t reg_x1;
} regstate;

#endif