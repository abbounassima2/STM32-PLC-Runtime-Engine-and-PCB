#ifndef PLC_SCHEDULER_H
#define PLC_SCHEDULER_H
/*
 * plc_scheduler.h
 * ---------------
 * Owns PLC mode (STOP/STARTUP/RUN) and the scan cycle. This is the
 * "platform" core - it should not need to change when you write a
 * different control application (that goes in user_program.c).
 */

#include <stdint.h>
#include "main.h"

typedef enum { PLC_STOP, PLC_STARTUP, PLC_RUN } PLC_Mode_t;

extern PLC_Mode_t current_mode;
extern PLC_Mode_t previous_mode;
extern const char *const mode_names[3];

extern uint32_t last_scan_time;
extern volatile uint8_t ob80_fired;
extern volatile uint8_t ob80_enable;
extern volatile uint8_t ob30_enable;

/* One-time transition-to-STOP actions (outputs safe-state, retentive save) */
void stop(void);

/* Organization-block style scheduler functions */
void OB100(void);   /* calls OB100_User() once on STARTUP entry */
void OB1(void);      /* main scan cycle: input -> user logic -> output, timed */
void scan(void);     /* the actual read/logic/write sequence inside OB1() */

/* RTOS task bodies */
void state_machine(void *argument);  /* mode sequencing + main scan (Normal prio) */
void OB30_Task(void *argument);      /* fast periodic task (High prio) */

/* TIM2 overrun-watchdog ISR hook, called from HAL_TIM_PeriodElapsedCallback */
void OB80_ISR_Handler(void);

#endif /* PLC_SCHEDULER_H */
