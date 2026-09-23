#ifndef PROCESS_IMAGE_H
#define PROCESS_IMAGE_H
/*
 * process_image.h
 * ----------------
 * The PLC-style process image: the tables that user logic actually
 * reads/writes (via the din1/dout1/ain1/aout1 style tag macros below),
 * and the two functions that move data between the tables and real
 * hardware.
 *
 * Ownership rule (see analysis, Finding #5 and "Process Image"
 * component): update_input() should be the ONLY writer of DI_table/
 * AI_table, and update_output() the ONLY reader of DQ_table/AQ_table
 * for the purpose of driving hardware. Anything that needs cross-task
 * access to these tables (e.g. a fast task like OB30) should go
 * through an explicit critical section - this header does not yet
 * enforce that; it is flagged here as a TODO rather than silently
 * added, since it changes concurrency behavior.
 */

#include "plc_config.h"
#include <stdint.h>

extern uint8_t  DI_table[NUM_DIN];
extern uint8_t  DQ_table[NUM_DOUT];
extern uint16_t AI_table[NUM_AIN];
extern uint16_t AQ_table[NUM_AOUT];

/* Raw DMA-updated holder for analog inputs (currently unused by
 * update_input(), which polls instead - kept for parity with the
 * original code / a future DMA-based input path). */
extern uint16_t AIN_HOLDER[NUM_AIN];

/* Reads real hardware into DI_table / AI_table. This is the function
 * that was previously commented out of the scan cycle (Finding #1) -
 * make sure it is actually called in production builds. */
void update_input(void);

/* Writes DQ_table / AQ_table out to real hardware (GPIO + DAC + PWM),
 * and reports scan diagnostics. */
void update_output(void);

/* DAC/PWM portion of update_output(), split out for testability. */
void To_DAC(void);

/* ---- Tag aliases: what user logic (OB*_User) actually writes ---- */
#define din1  DI_table[0]
#define din2  DI_table[1]
#define din3  DI_table[2]
#define din4  DI_table[3]
#define din5  DI_table[4]
#define din6  DI_table[5]
#define din7  DI_table[6]
#define din8  DI_table[7]

#define dout1 DQ_table[0]
#define dout2 DQ_table[1]
#define dout3 DQ_table[2]
#define dout4 DQ_table[3]
#define dout5 DQ_table[4]
#define dout6 DQ_table[5]
#define dout7 DQ_table[6]
#define dout8 DQ_table[7]

#define ain1  AI_table[0]
#define ain2  AI_table[1]
#define ain3  AI_table[2]
#define ain4  AI_table[3]

#define aout1 AQ_table[0]
#define aout2 AQ_table[1]
#define aout3 AQ_table[2]
#define aout4 AQ_table[3]

#endif /* PROCESS_IMAGE_H */
