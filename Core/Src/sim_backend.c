#include "sim_backend.h"

#ifdef PLC_SIMULATION

#include "process_image.h"
#include "plc_scheduler.h"
#include <math.h>

static float simulation_t = 0.0f;

typedef enum { PWR_NORMAL, PWR_LOSS, PWR_RESTORED } power_state_t;
static power_state_t power_state = PWR_NORMAL;
static uint8_t power_on = 1;
static uint32_t power_off_start = 0;

void update_physical_pins(void)
{
    /* Synthetic analog input: a slow sine wave scaled 0..4095 */
    simulation_t += 0.1f;
    ain1 = (uint16_t)((sinf(simulation_t) + 1.0f) * 0.5f * 4095.0f);

    /* Scripted power-loss / cold-restart scenario, for exercising
     * STOP/STARTUP/RUN transitions and retentive save/load under
     * Renode without real hardware. */
    switch (power_state) {
        case PWR_NORMAL:
            if (simulation_t >= 25.0f) {
                power_state = PWR_LOSS;
                power_on = 0;
                power_off_start = HAL_GetTick();
                for (int i = 0; i < NUM_DIN; i++)  DI_table[i] = 0;
                for (int i = 0; i < NUM_AIN; i++)  AI_table[i] = 0;
            }
            break;

        case PWR_LOSS:
            if ((HAL_GetTick() - power_off_start) > 2000) {
                power_state = PWR_RESTORED;
                power_on = 1;
            }
            break;

        case PWR_RESTORED:
            /* stays powered; scenario complete */
            break;
    }
}

#endif /* PLC_SIMULATION */
