#include "io_config.h"

/* Digital inputs */
PinMap_t din_map[NUM_DIN] = {
    {TYPE_DIN, GPIOE, GPIO_PIN_8,  0, 0, "din1"},
    {TYPE_DIN, GPIOE, GPIO_PIN_10, 0, 1, "din2"},
    {TYPE_DIN, GPIOE, GPIO_PIN_12, 0, 2, "din3"},
    {TYPE_DIN, GPIOE, GPIO_PIN_9,  0, 3, "din4"},
    {TYPE_DIN, GPIOE, GPIO_PIN_11, 0, 4, "din5"},
    {TYPE_DIN, GPIOE, GPIO_PIN_13, 0, 5, "din6"},
    {TYPE_DIN, GPIOD, GPIO_PIN_3,  0, 6, "din7"},
    {TYPE_DIN, GPIOD, GPIO_PIN_1,  0, 7, "din8"},
};

/* Digital outputs (relays, LEDs) */
PinMap_t dout_map[NUM_DOUT] = {
    {TYPE_DOUT, GPIOC, GPIO_PIN_6,  0, 0, "dout1"},
    {TYPE_DOUT, GPIOD, GPIO_PIN_14, 0, 1, "dout2"},
    {TYPE_DOUT, GPIOC, GPIO_PIN_8,  0, 2, "dout3"},
    {TYPE_DOUT, GPIOC, GPIO_PIN_12, 0, 3, "dout4"},
    {TYPE_DOUT, GPIOA, GPIO_PIN_8,  0, 4, "dout5"},
    {TYPE_DOUT, GPIOA, GPIO_PIN_10, 0, 5, "dout6"},
    {TYPE_DOUT, GPIOA, GPIO_PIN_12, 0, 6, "dout7"},
    {TYPE_DOUT, GPIOC, GPIO_PIN_12, 0, 7, "dout8"},
    /* NOTE (carried over from original code, not fixed here):
     * dout4 and dout8 both map to GPIOC/GPIO_PIN_12. Confirm whether
     * this is intentional (two logical channels driving one physical
     * pin) or a copy/paste error before relying on dout8. */
};

/* Analog inputs */
PinMap_t ain_map[NUM_AIN] = {
    {TYPE_AIN, GPIOA, GPIO_PIN_0, ADC_CHANNEL_0,  0, "ain1"},
    {TYPE_AIN, GPIOA, GPIO_PIN_1, ADC_CHANNEL_1,  1, "ain2"},
    {TYPE_AIN, GPIOC, GPIO_PIN_2, ADC_CHANNEL_12, 2, "ain3"},
    {TYPE_AIN, GPIOC, GPIO_PIN_3, ADC_CHANNEL_13, 3, "ain4"},
};

/* Analog outputs (DAC channels 1-2, PWM/TIM12 channels for 3-4) */
PinMap_t aout_map[NUM_AOUT] = {
    {TYPE_AOUT, GPIOA, GPIO_PIN_4,  DAC_CHANNEL_1, 0, "aout1"},
    {TYPE_AOUT, GPIOA, GPIO_PIN_5,  DAC_CHANNEL_2, 1, "aout2"},
    {TYPE_AOUT, GPIOB, GPIO_PIN_14, TIM_CHANNEL_1, 2, "aout3"},
    {TYPE_AOUT, GPIOB, GPIO_PIN_15, TIM_CHANNEL_2, 3, "aout4"},
};
