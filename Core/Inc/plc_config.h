#ifndef PLC_CONFIG_H
#define PLC_CONFIG_H
/*
 * plc_config.h
 * ------------
 * All tunable constants in one place. Previously these were scattered
 * across final_plc.c and final_plc.h (and duplicated between the two).
 */

/* --- scan cycle timing (see plc_scheduler.c) --- */
#define MIN_SCAN_TIME_MS   5
#define MAX_SCAN_TIME_MS   150

/* --- I/O channel counts --- */
#define NUM_AIN   4
#define NUM_AOUT  4
#define NUM_DIN   8
#define NUM_DOUT  8

/* --- retentive memory --- */
#define MAX_RETENTIVE_TAGS    32
#define FLASH_RETENTIVE_ADDR  0x080E0000U
#define RETENTIVE_BUF_SIZE    512   /* must stay in sync with the buffer
                                        used in retentive.c; Config_RegisterTag
                                        style bounds-checking should be added
                                        here eventually (see analysis, Finding #10 /
                                        ADR "Config/Retentive Manager"). */

/* --- RS-485 comms --- */
#define RS485_BUF_SIZE       256
#define RS485_FRAME_TIMEOUT_MS 50

/*
 * PLC_SIMULATION
 * --------------
 * Define this at the project/build level (e.g. as a compiler -D flag)
 * to compile in the synthetic analog-input generator + scripted
 * power-loss scenario from sim_backend.c. Leave UNDEFINED for a
 * production build that reads real hardware inputs via update_input().
 *
 * This is the fix for Finding #1 from the code analysis: previously
 * the simulation code ran unconditionally and real update_input() was
 * commented out, so production builds never read real inputs.
 *
 * #define PLC_SIMULATION
 */

#endif /* PLC_CONFIG_H */
