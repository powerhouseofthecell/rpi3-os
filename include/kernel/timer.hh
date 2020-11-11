#ifndef TIMER_HH
#define TIMER_HH
#include "common/types.hh"

#define PERIPHERAL_BASE     ((uint64_t)0x3F000000)
#define IRQ_BASIC_PENDING	(PERIPHERAL_BASE+0x0000B200)
#define IRQ_PENDING_1		(PERIPHERAL_BASE+0x0000B204)
#define IRQ_PENDING_2		(PERIPHERAL_BASE+0x0000B208)
#define FIQ_CONTROL		    (PERIPHERAL_BASE+0x0000B20C)
#define ENABLE_IRQS_1		(PERIPHERAL_BASE+0x0000B210)
#define ENABLE_IRQS_2		(PERIPHERAL_BASE+0x0000B214)
#define ENABLE_BASIC_IRQS	(PERIPHERAL_BASE+0x0000B218)
#define DISABLE_IRQS_1		(PERIPHERAL_BASE+0x0000B21C)
#define DISABLE_IRQS_2		(PERIPHERAL_BASE+0x0000B220)
#define DISABLE_BASIC_IRQS	(PERIPHERAL_BASE+0x0000B224)

#define SYSTEM_TIMER_IRQ_0	(1 << 0)
#define SYSTEM_TIMER_IRQ_1	(1 << 1)
#define SYSTEM_TIMER_IRQ_2	(1 << 2)
#define SYSTEM_TIMER_IRQ_3	(1 << 3)

#define TIMER_CS        (PERIPHERAL_BASE+0x00003000)
#define TIMER_CLO       (PERIPHERAL_BASE+0x00003004)
#define TIMER_CHI       (PERIPHERAL_BASE+0x00003008)
#define TIMER_C0        (PERIPHERAL_BASE+0x0000300C)
#define TIMER_C1        (PERIPHERAL_BASE+0x00003010)
#define TIMER_C2        (PERIPHERAL_BASE+0x00003014)
#define TIMER_C3        (PERIPHERAL_BASE+0x00003018)

#define TIMER_CS_M0	(1 << 0)
#define TIMER_CS_M1	(1 << 1)
#define TIMER_CS_M2	(1 << 2)
#define TIMER_CS_M3	(1 << 3)

// key addresses for the arm timer
#define ARM_TIMER_LOAD      (PERIPHERAL_BASE+0x0000B400)
#define ARM_TIMER_VALUE     (PERIPHERAL_BASE+0x0000B404)
#define ARM_TIMER_CONTROL   (PERIPHERAL_BASE+0x0000B408)
#define ARM_TIMER_IRQ_CLEAR (PERIPHERAL_BASE+0x0000B40C)

// key values for the local timer
#define LOCAL_TIMER             0x40000034
#define LOCAL_TIMER_ENABLE      (1<<28)
#define LOCAL_TIMER_IRQ_ENABLE  (1<<29)
#define LOCAL_TIMER_RELOAD      (1<<31)

// the frequency with which the timer goes off, in hertz
#define HZ 100

typedef struct {
    uint32_t control_status;
    uint32_t clear_reload;
} local_timer_t;

extern uint64_t ticks;

// enables timer interrupts
void init_interrupts();

#endif