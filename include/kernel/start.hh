#ifndef START_HH
#define START_HH

// sctlr_el1 constants (D13.2.113)
#define SCTLR_LSMAOE        (1<<29)
#define SCTLR_nTLSMD        (1<<28)
#define SCTLR_SPAN          (1<<23)
#define SCTLR_EIS           (1<<22)
#define SCTLR_TSCXT         (1<<20)
#define SCTLR_EOS           (1<<11)
#define SCTLR_MMU           (1<<0)
#define SCTLR_EL1_VAL       (SCTLR_LSMAOE | SCTLR_nTLSMD | SCTLR_SPAN | SCTLR_EIS | SCTLR_TSCXT | SCTLR_EOS)

// hcr_el2 constants (D13.2.47)
#define HCR_RW              (1<<31)
#define HCR_EL2_VAL         (HCR_RW)

// spsr_el2 constants (C5.2.18)
#define SPSR_DAIF           (15<<6)
#define SPSR_M_EL1h         (5<<0)
#define SPSR_EL2_VAL        (SPSR_DAIF | SPSR_M_EL1h)

#endif