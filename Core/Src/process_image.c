#include "process_image.h"
#include "io_config.h"
#include "board.h"
#include "plc_scheduler.h"
#include "user_program.h"
#include <stdio.h>

uint8_t  DI_table[NUM_DIN];
uint8_t  DQ_table[NUM_DOUT];
uint16_t AI_table[NUM_AIN];
uint16_t AQ_table[NUM_AOUT];
uint16_t AIN_HOLDER[NUM_AIN];

void update_input(void)
{
    /* Digital inputs */
    for (int i = 0; i < NUM_DIN; i++) {
        DI_table[i] = (uint8_t)HAL_GPIO_ReadPin(din_map[i].port, din_map[i].pin);
    }

    /* Analog inputs - reconfigure + convert one channel at a time */
    for (int i = 0; i < NUM_AIN; i++) {
        ADC_ChannelConfTypeDef sConfig = {0};
        sConfig.Channel = ain_map[i].channel;
        sConfig.Rank = 1;
        sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;

        if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK) {
            AI_table[i] = 0;
            continue;
        }

        HAL_ADC_Start(&hadc1);
        if (HAL_ADC_PollForConversion(&hadc1, 10) == HAL_OK) {
            AI_table[i] = (uint16_t)HAL_ADC_GetValue(&hadc1);
        } else {
            AI_table[i] = 0; /* fallback on timeout */
        }
        HAL_ADC_Stop(&hadc1);
    }
}

void To_DAC(void)
{
    for (uint8_t i = 0; i < NUM_AOUT; i++) {
        uint32_t channel;
        uint16_t raw_val = AQ_table[i];

        channel = (i == 0) ? DAC_CHANNEL_1 : DAC_CHANNEL_2;

        if (raw_val > 4095) {
            raw_val = 4095;
        }

        HAL_DAC_SetValue(&hdac, channel, DAC_ALIGN_12B_R, raw_val);
    }
    __HAL_TIM_SET_COMPARE(&htim12, TIM_CHANNEL_1, AQ_table[2]);
    __HAL_TIM_SET_COMPARE(&htim12, TIM_CHANNEL_2, AQ_table[3]);
}


void update_output(void)
{
    for (uint8_t i = 0; i < NUM_DOUT; i++) {
        HAL_GPIO_WritePin(dout_map[i].port, dout_map[i].pin,
                           (DQ_table[dout_map[i].index] == 0) ? GPIO_PIN_RESET
                                                               : GPIO_PIN_SET);
    }

    To_DAC();

    /*
     * TODO (flagged, not fixed here - see analysis Finding #4):
     * this printf() -> HAL_UART_Transmit(..., HAL_MAX_DELAY) is a
     * BLOCKING, UNBOUNDED call inside the scan cycle. If UART4 ever
     * stalls, the whole scan cycle hangs. Move this to a queued /
     * non-blocking Diagnostics Manager before shipping to real
     * hardware. Left as-is here since changing it changes runtime
     * behavior and wasn't part of the agreed low-risk fix set.
     */
    printf(
        "[PLC] Mode=%s  AI1=%u   PID_OUT=%u  AQ2=%u  Alarm=%u  Count=%u  Scan_time=%lu ms,OB80_flag=%d\r\n",
        mode_names[current_mode],
        AI_table[0],
        AQ_table[0],
        AQ_table[1],
        DQ_table[1],
        count,
        (unsigned long)last_scan_time, ob80_fired);
}
