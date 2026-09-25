#include "plc_scheduler.h"
#include "process_image.h"
#include "sim_backend.h"
#include "retentive.h"
#include "user_program.h"
#include "board.h"
#include "plc_config.h"
#include "cmsis_os.h"

PLC_Mode_t current_mode = PLC_STOP;
PLC_Mode_t previous_mode = PLC_STOP;
const char *const mode_names[3] = {"STOP", "STARTUP", "RUN"};

uint32_t last_scan_time = 0;
volatile uint8_t ob80_fired = 0;
volatile uint8_t ob80_enable = 0;
volatile uint8_t ob30_enable = 0;

void stop(void)
{
    for (int i = 0; i < NUM_DOUT; i++) DQ_table[i] = 0;
    for (int i = 0; i < NUM_AOUT; i++) AQ_table[i] = 0;
    update_output();
    PLC_Retentive_Save();
}

void OB100(void)
{
    OB100_User();
    PLC_Retentive_Load();
}

void scan(void)
{
    update_physical_pins();   // for simulated behavior
    update_input();           // real physical pins
    OB1_User();
    update_output();
}

void OB1(void)
{
    uint32_t cycle_start = HAL_GetTick();

    ob80_enable = 1;
    HAL_TIM_Base_Start_IT(&htim2);

    scan();

    HAL_TIM_Base_Stop_IT(&htim2);
    ob80_enable = 0;

    last_scan_time = HAL_GetTick() - cycle_start;

    if (last_scan_time < MIN_SCAN_TIME_MS) {

        osDelay(MIN_SCAN_TIME_MS - last_scan_time);
    }
}

void OB80_ISR_Handler(void)
{
    if (ob80_enable) {
        ob80_fired = 1;
        OB80_User();
    }
}

void state_machine(void *argument)
{
    (void)argument;

    for (;;) {

        uint8_t Q = 1;

        previous_mode = current_mode;

        switch (current_mode) {
            case PLC_STOP:
                if (Q) {
                    current_mode = PLC_STARTUP;
                } else {
                    stop();
                    if (previous_mode == PLC_RUN) {
                        PLC_Retentive_Save();
                    }
                }
                break;

            case PLC_STARTUP:
                OB100();
                osDelay(500);
                current_mode = PLC_RUN;
                break;

            case PLC_RUN:
                if (!Q) {
                    current_mode = PLC_STOP;
                } else {
                    OB1();
                }
                break;
        }

        osDelay(100);
    }
}

void OB30_Task(void *argument)
{
    (void)argument;
    for (;;) {
        if (ob30_enable) {
            OB30_User();
        }
        osDelay(100);
    }
}
