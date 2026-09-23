#ifndef USER_PROGRAM_H
#define USER_PROGRAM_H
/*
 * user_program.h
 * ---------------
 * Application layer: the actual control logic for this machine
 * (level-sensor alarm + counter, PID loop). This is the file that
 * changes when the CONTROL TASK changes, as opposed to io_config.c
 * (changes when the BOARD changes) or the platform modules (which
 * shouldn't change per-project at all).
 */

#include <stdint.h>

/* Exposed for diagnostics (process_image.c status line). If this
 * needs to be shared more widely, it belongs in a proper tag/config
 * table (see analysis, "Config/Retentive Manager") rather than as a
 * loose extern - kept as extern here to match original behavior. */
extern uint8_t count;

/* PLC organization-block style entry points, called by plc_scheduler.c */
void OB100_User(void);  /* one-time init / retentive tag registration */
void OB1_User(void);    /* main scan cycle logic */
void OB30_User(void);   /* fast periodic logic (PID) */
void OB80_User(void);   /* scan-overrun ISR hook (see plc_scheduler.c) */

#endif /* USER_PROGRAM_H */
