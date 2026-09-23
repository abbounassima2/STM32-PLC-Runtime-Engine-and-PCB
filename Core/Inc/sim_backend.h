#ifndef SIM_BACKEND_H
#define SIM_BACKEND_H
/*
 * sim_backend.h
 * -------------
 * Synthetic input generator + scripted power-loss/cold-restart
 * scenario, for bench/Renode development ONLY.
 *
 * This entire module compiles to nothing unless PLC_SIMULATION is
 * defined (see plc_config.h). This is the fix for Finding #1: the
 * original code called this unconditionally and had the real
 * update_input() commented out, so production builds never read real
 * hardware. Now the choice is explicit and made at compile time.
 */

#include "plc_config.h"

#ifdef PLC_SIMULATION
void update_physical_pins(void);
#else
/* No-op in production builds so call sites don't need #ifdef guards. */
static inline void update_physical_pins(void) {}
#endif

#endif /* SIM_BACKEND_H */
