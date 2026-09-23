#ifndef API_H
#define API_H
/*
 * api.h
 * -----
 * IEC-61131-style function-block library: latches, timers, counters,
 * edge detectors, comparators, scaling, math, and a PID block.
 *
 * This module is hardware-independent aside from PLC_TIME_MS, and is
 * one of the pieces of the original codebase that is already suitable
 * as reusable "platform" code (see analysis, section "PLC suitability").
 *
 * CHANGE LOG vs. original:
 *  - PID_Compact_Run: added anti-windup clamp on the integral
 *    accumulator (Finding #7). Previously only the final output was
 *    clamped; the integral term itself could wind up unbounded during
 *    sustained large error.
 */

#include <stdint.h>
#include <stdbool.h>

#include "cmsis_os2.h"
#define PLC_TIME_MS (osKernelGetTickCount())
/*
 * TODO (future, not applied here): make the time source injectable
 * (e.g. a weak PLC_GetTimeMs() or function pointer) instead of hard-
 * wiring osKernelGetTickCount(), so this library can run under a PC
 * simulator/Renode harness without an RTOS. See analysis, "Timer/
 * Counter Engine" section.
 */

/* ---------------------------------------------------------------- */
/* Contacts                                                          */
/* ---------------------------------------------------------------- */
static inline bool NO(bool signal) { return signal; }
static inline bool NC(bool signal) { return !signal; }

/* ---------------------------------------------------------------- */
/* Comparators                                                       */
/* ---------------------------------------------------------------- */
static inline uint8_t EQ_I(int in1, int in2) { return (in1 == in2) ? 1 : 0; }
static inline uint8_t GE_R(float in1, float in2) { return (in1 >= in2) ? 1 : 0; }
static inline uint8_t LT_R(float in1, float in2) { return (in1 < in2) ? 1 : 0; }

/* ---------------------------------------------------------------- */
/* Math                                                               */
/* ---------------------------------------------------------------- */
static inline int ADD_I(int in1, int in2) { return in1 + in2; }

static inline float DIV_R(float in1, float in2)
{
    if (in2 == 0.0f) return 0.0f;   /* Siemens-style: 0 on div-by-zero */
    return in1 / in2;
}

/* ---------------------------------------------------------------- */
/* Scaling                                                            */
/* ---------------------------------------------------------------- */
static inline float SCALE_X(float value, float min, float max)
{
    if (value < 0.0f)    value = 0.0f;
    if (value > 4095.0f) value = 4095.0f;
    return ((value / 4095.0f) * (max - min)) + min;
}

static inline uint16_t UNSCALE_X(float percent)
{
    if (percent < 0.0f)   percent = 0.0f;
    if (percent > 100.0f) percent = 100.0f;
    return (uint16_t)((percent / 100.0f) * 4095.0f);
}

static inline float MOVE(float in) { return in; }

static inline float MUX(uint8_t K, float IN0, float IN1, float IN2, float IN3)
{
    switch (K) {
        case 0:  return IN0;
        case 1:  return IN1;
        case 2:  return IN2;
        case 3:  return IN3;
        default: return IN0; /* Siemens fallback: out-of-range -> IN0 */
    }
}

/* ================================================================ */
/* Latches                                                            */
/* ================================================================ */

/* --- SR (Set-dominant) --- */
typedef struct { bool Q; } SR_Instance;

static inline bool SR_Run(SR_Instance *inst, bool S, bool R)
{
    if (S) inst->Q = 1;
    if (R) inst->Q = 0;
    return inst->Q;
}

#define SR(S, R) \
    ({ static SR_Instance inst = {0}; SR_Run(&inst, S, R); })

/* --- RS (Reset-dominant) --- */
typedef struct { bool Q; } RS_Instance;

static inline bool RS_Run(RS_Instance *inst, bool S, bool R)
{
    if (R) inst->Q = 1;
    if (S) inst->Q = 0;
    return inst->Q;
}

#define RS(S, R) \
    ({ static RS_Instance inst = {0}; RS_Run(&inst, S, R); })

/* ================================================================ */
/* Timers                                                             */
/* ================================================================ */

/* --- TON: on-delay --- */
typedef struct {
    bool IN; uint32_t PT;
    bool Q;  uint32_t ET;
    uint32_t start_time;
    bool running;
} TON_Instance;

static inline void TON_Run(TON_Instance *inst)
{
    if (inst->IN) {
        if (!inst->running) {
            inst->running = true;
            inst->start_time = PLC_TIME_MS;
        }
        inst->ET = PLC_TIME_MS - inst->start_time;
        if (inst->ET >= inst->PT) {
            inst->Q = true;
            inst->ET = inst->PT;
        } else {
            inst->Q = false;
        }
    } else {
        inst->Q = false;
        inst->ET = 0;
        inst->running = false;
    }
}

#define TON(NAME, IN_VALUE, PT_VALUE) \
    do { NAME.IN = IN_VALUE; NAME.PT = PT_VALUE; TON_Run(&NAME); } while (0)

/* --- TOF: off-delay --- */
typedef struct {
    bool IN; uint32_t PT;
    bool Q;  uint32_t ET;
    uint32_t start_time;
    bool timing;
} TOF_Instance;

static inline void TOF_Run(TOF_Instance *inst)
{
    if (inst->IN) {
        inst->Q = true;
        inst->ET = 0;
        inst->timing = false;
    } else {
        if (inst->Q && !inst->timing) {
            inst->timing = true;
            inst->start_time = PLC_TIME_MS;
        }
        if (inst->timing) {
            inst->ET = PLC_TIME_MS - inst->start_time;
            if (inst->ET >= inst->PT) {
                inst->Q = false;
                inst->ET = inst->PT;
                inst->timing = false;
            }
        }
    }
}

#define TOF(NAME, IN_VALUE, PT_VALUE) \
    do { NAME.IN = IN_VALUE; NAME.PT = PT_VALUE; TOF_Run(&NAME); } while (0)

/* --- TP: pulse --- */
typedef struct {
    bool IN; uint32_t PT;
    bool Q;  uint32_t ET;
    bool prev_IN;
    uint32_t start_time;
    bool running;
} TP_Instance;

static inline void TP_Run(TP_Instance *inst)
{
    bool rising_edge = (inst->IN && !inst->prev_IN);
    inst->prev_IN = inst->IN;

    if (rising_edge) {
        inst->running = true;
        inst->start_time = PLC_TIME_MS;
        inst->Q = true;
    }

    if (inst->running) {
        inst->ET = PLC_TIME_MS - inst->start_time;
        if (inst->ET >= inst->PT) {
            inst->Q = false;
            inst->ET = inst->PT;
            inst->running = false;
        }
    } else {
        inst->ET = 0;
    }
}

#define TP(NAME, IN_VALUE, PT_VALUE) \
    do { NAME.IN = IN_VALUE; NAME.PT = PT_VALUE; TP_Run(&NAME); } while (0)

/* ================================================================ */
/* Counters                                                           */
/* ================================================================ */

/* --- CTU: count up --- */
typedef struct {
    bool CU; bool R; int32_t PV;
    bool Q;  int32_t CV;
    bool prev_CU;
} CTU_Instance;

static inline void CTU_Run(CTU_Instance *inst)
{
    bool rising_edge = (inst->CU && !inst->prev_CU);
    inst->prev_CU = inst->CU;

    if (inst->R) {
        inst->CV = 0;
    } else if (rising_edge) {
        inst->CV++;
    }
    inst->Q = (inst->CV >= inst->PV);
}

#define CTU(NAME, CU_VALUE, R_VALUE, PV_VALUE) \
    do { NAME.CU = CU_VALUE; NAME.R = R_VALUE; NAME.PV = PV_VALUE; CTU_Run(&NAME); } while (0)

/* --- CTD: count down --- */
typedef struct {
    bool CD; bool LD; int32_t PV;
    bool Q;  int32_t CV;
    bool prev_CD;
} CTD_Instance;

static inline void CTD_Run(CTD_Instance *inst)
{
    bool rising_edge = (inst->CD && !inst->prev_CD);
    inst->prev_CD = inst->CD;

    if (inst->LD) {
        inst->CV = inst->PV;
    } else if (rising_edge) {
        inst->CV--;
    }
    inst->Q = (inst->CV <= 0);
}

#define CTD(NAME, CD_VALUE, LD_VALUE, PV_VALUE) \
    do { NAME.CD = CD_VALUE; NAME.LD = LD_VALUE; NAME.PV = PV_VALUE; CTD_Run(&NAME); } while (0)

/* --- CTUD: count up/down --- */
typedef struct {
    bool CU; bool CD; bool R; bool LD; int32_t PV;
    bool QU; bool QD; int32_t CV;
    bool prev_CU; bool prev_CD;
} CTUD_Instance;

static inline void CTUD_Run(CTUD_Instance *inst)
{
    bool cu_rising = (inst->CU && !inst->prev_CU);
    bool cd_rising = (inst->CD && !inst->prev_CD);
    inst->prev_CU = inst->CU;
    inst->prev_CD = inst->CD;

    if (inst->R) {
        inst->CV = 0;
    } else if (inst->LD) {
        inst->CV = inst->PV;
    } else {
        if (cu_rising && !cd_rising) inst->CV++;
        if (cd_rising && !cu_rising) inst->CV--;
    }

    inst->QU = (inst->CV >= inst->PV);
    inst->QD = (inst->CV <= 0);
}

#define CTUD(NAME, CU_VALUE, CD_VALUE, R_VALUE, LD_VALUE, PV_VALUE) \
    do { \
        NAME.CU = CU_VALUE; NAME.CD = CD_VALUE; \
        NAME.R = R_VALUE;   NAME.LD = LD_VALUE; \
        NAME.PV = PV_VALUE; CTUD_Run(&NAME); \
    } while (0)

/* ================================================================ */
/* Edge detection                                                     */
/* ================================================================ */
typedef struct { bool CLK; bool Q; bool prev_CLK; } R_TRIG_Instance;

static inline void R_TRIG_Run(R_TRIG_Instance *inst)
{
    inst->Q = (inst->CLK && !inst->prev_CLK);
    inst->prev_CLK = inst->CLK;
}
#define R_TRIG(NAME, CLK_VALUE) \
    do { NAME.CLK = CLK_VALUE; R_TRIG_Run(&NAME); } while (0)

typedef struct { bool CLK; bool Q; bool prev_CLK; } F_TRIG_Instance;

static inline void F_TRIG_Run(F_TRIG_Instance *inst)
{
    inst->Q = (!inst->CLK && inst->prev_CLK);
    inst->prev_CLK = inst->CLK;
}
#define F_TRIG(NAME, CLK_VALUE) \
    do { NAME.CLK = CLK_VALUE; F_TRIG_Run(&NAME); } while (0)

/* ================================================================ */
/* Boolean logic                                                      */
/* ================================================================ */
static inline bool AND(bool a, bool b) { return a && b; }
static inline bool OR(bool a, bool b)  { return a || b; }
static inline bool XOR(bool a, bool b) { return (a && !b) || (!a && b); }
static inline bool NOT(bool a)         { return !a; }

/* ================================================================ */
/* PID                                                                 */
/* ================================================================ */
typedef struct {
    /* Universal block pins */
    bool  EN;
    float Setpoint;
    float Input;
    float Gain;    /* Kp */
    float Ti;      /* seconds */
    float Td;      /* seconds */
    float Output;  /* 0..100 */
    bool  ENO;

    /* Internal state */
    uint32_t Internal_LastTime;
    float    Internal_Accumulator;
    float    Internal_LastInput;
} PID_Compact_Instance;

/* Integral accumulator clamp. The accumulator is expressed in the
 * same units as Output (0..100), so clamping it to the output range
 * (with a little headroom) prevents windup while still allowing the
 * I-term to dominate briefly during large sustained error. Tune if
 * your process needs a different margin. */
#define PID_INTEGRAL_CLAMP_MIN   0.0f
#define PID_INTEGRAL_CLAMP_MAX   100.0f

static inline void PID_Compact_Run(PID_Compact_Instance *inst)
{
    if (!inst->EN) {
        inst->ENO = false;
        return;
    }
    inst->ENO = true;

    uint32_t now = PLC_TIME_MS;

    if (inst->Internal_LastTime == 0) {
        inst->Internal_LastTime = now;
        inst->Internal_LastInput = inst->Input;
    }

    float dt = (now - inst->Internal_LastTime) / 1000.0f;
    if (dt <= 0.0f) dt = 0.01f;
    inst->Internal_LastTime = now;

    float error = inst->Setpoint - inst->Input;
    float P = inst->Gain * error;

    /* --- I term, WITH anti-windup clamp (fix for Finding #7) --- */
    if (inst->Ti > 0.0f) {
        inst->Internal_Accumulator += (inst->Gain / inst->Ti) * error * dt;

        if (inst->Internal_Accumulator > PID_INTEGRAL_CLAMP_MAX)
            inst->Internal_Accumulator = PID_INTEGRAL_CLAMP_MAX;
        if (inst->Internal_Accumulator < PID_INTEGRAL_CLAMP_MIN)
            inst->Internal_Accumulator = PID_INTEGRAL_CLAMP_MIN;
    }
    float I = inst->Internal_Accumulator;

    /* --- D term --- */
    float D = 0.0f;
    if (inst->Td > 0.0f && dt > 0.0f) {
        D = inst->Gain * inst->Td * (inst->Input - inst->Internal_LastInput) / dt;
    }
    inst->Internal_LastInput = inst->Input;

    float out = P + I + D;

    /* --- output clamp --- */
    if (out > 100.0f) out = 100.0f;
    if (out < 0.0f)   out = 0.0f;

    inst->Output = out;
}

#define PID_COMPACT(NAME, EN_IN, SP, PV, KP, TI, TD, OUT_VAR, ENO_OUT) \
    do { \
        NAME.EN       = (EN_IN); \
        NAME.Setpoint = (SP); \
        NAME.Input    = (PV); \
        NAME.Gain     = (KP); \
        NAME.Ti       = (TI); \
        NAME.Td       = (TD); \
        PID_Compact_Run(&NAME); \
        (OUT_VAR)     = NAME.Output; \
        (ENO_OUT)     = NAME.ENO; \
    } while (0)

/* ================================================================ */
/* User program entry points (weak-style)                            */
/* ================================================================ */
void OB100_User(void);
void OB1_User(void);
void OB30_User(void);
void OB80_User(void);

#endif /* API_H */
