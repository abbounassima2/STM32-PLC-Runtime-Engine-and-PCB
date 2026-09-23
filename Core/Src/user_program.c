/*
 * user_program.c
 * ===============
 * >>> THIS IS THE FILE YOU EDIT TO WRITE YOUR PLC PROGRAM. <<<
 *
 * Everything else in src/ and inc/ is platform/runtime code that
 * shouldn't normally need to change when you write a new control
 * application. This file is the application layer: it's where the
 * example level-sensor-alarm and PID logic live, and where you'd add
 * or replace logic for your own machine.
 *
 * Available to you here:
 *   - Tag macros from process_image.h: din1..din8, dout1..dout8,
 *     ain1..ain4, aout1..aout4
 *   - Function blocks from api.h: TON, TOF, TP, CTU, CTD, CTUD,
 *     R_TRIG, F_TRIG, SR, RS, PID_COMPACT, comparators, scaling, math
 *   - SET_RETENTIVE(var) from retentive.h, to persist a variable
 *     across STOP/power cycles - call it inside OB100_User() only.
 *
 * Called by plc_scheduler.c:
 *   OB100_User()  - once, when entering STARTUP. Register retentive
 *                   tags here.
 *   OB1_User()    - once per main scan cycle (RUN mode).
 *   OB30_User()   - once per OB30 fast-task tick (~100 ms), independent
 *                   of the main scan.
 */

#include "user_program.h"
#include "process_image.h"
#include "retentive.h"
#include "api.h"
#include <stdbool.h>


uint8_t count = 0;

/* --- Example: level-sensor threshold -> alarm -> counter --- */
static CTU_Instance ctu;
static TON_Instance alarm_delay;

/* --- Example: PID loop instance for OB30 --- */
static PID_Compact_Instance pid;
static float scaled_value;

void OB100_User(void)
{
    /* Register anything that must survive a STOP/power cycle here. */
    SET_RETENTIVE(count);
}

void OB1_User(void)
{
    /* Scale the raw ADC reading (0..4095) to an engineering range,
     * e.g. 0..100 for a level sensor in %. */
    scaled_value = SCALE_X((float)ain1, 0.0f, 100.0f);

    /* Debounce the high-level condition for 2 seconds before alarming. */
    TON(alarm_delay, GE_R(scaled_value, 80.0f), 2000);

    dout1 = alarm_delay.Q;  /* alarm output */

    /* Count each time the alarm activates (rising edge handled inside CTU). */
    CTU(ctu, alarm_delay.Q, /*reset=*/din2, 9999);
    count = (uint8_t)ctu.CV;  /* NOTE: CV is int32_t and can reach 9999;
                                 count is uint8_t and will wrap past 255.
                                 Left as original behavior - widen count's
                                 type if you need the full range. */
}

void OB30_User(void)
{
    bool eno;

    /* Setpoint 300 (engineering units), input = scaled level, Kp=2, Ti=10, Td=0 */
    PID_COMPACT(pid, /*EN=*/1, /*SP=*/300, /*PV=*/scaled_value,
                /*Kp=*/2, /*Ti=*/10, /*Td=*/0,
                /*OUT=*/aout2, /*ENO=*/eno);
    (void)eno;

    /* aout1: properly rescaled 0..4095 for the DAC (fix for Finding #6 -
     * the original code wrote the raw 0..100 PID output directly to
     * aout1, which meant it never exceeded ~2.4% of full scale). */
    aout1 = UNSCALE_X(pid.Output);

    dout1 = (pid.Output > 90.0f) ? 1 : 0;  /* high-output indicator */
}

void OB80_User(void)
{
    /* Called from the TIM2 scan-overrun ISR path (see plc_scheduler.c).
     * Keep this SHORT - it runs in interrupt context. Currently a
     * placeholder; add a diagnostics report here once the Diagnostics
     * Manager exists (see analysis). */
}
