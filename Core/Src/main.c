#include "main.h"
#include "cmsis_os.h"

#include "board.h"
#include "plc_scheduler.h"
#include "comm_rs485.h"
#include "retentive.h"

/* --- Peripheral handles (defined here, extern'd via board.h) --- */
ADC_HandleTypeDef  hadc1;
DAC_HandleTypeDef  hdac;
TIM_HandleTypeDef  htim2;
TIM_HandleTypeDef  htim12;
UART_HandleTypeDef huart4;
UART_HandleTypeDef huart3;

/* --- RTOS thread handles/attributes --- */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
    .name = "defaultTask",
    .stack_size = 512 * 4,
    .priority = (osPriority_t) osPriorityNormal,
};

osThreadId_t ob30TaskHandle;
const osThreadAttr_t ob30Task_attributes = {
    .name = "ob30Task",
    .stack_size = 512 * 4,
    .priority = (osPriority_t) osPriorityHigh,
};

osThreadId_t commTaskHandle;
const osThreadAttr_t commTask_attributes = {
    .name = "commTask",
    .stack_size = 256 * 4,
    .priority = (osPriority_t) osPriorityLow,
};


void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_ADC1_Init(void);
static void MX_DAC_Init(void);
static void MX_TIM2_Init(void);
static void MX_TIM12_Init(void);
static void MX_UART4_Init(void);
static void MX_USART3_UART_Init(void);

int main(void)
{
    HAL_Init();
    SystemClock_Config();

    MX_GPIO_Init();
    MX_ADC1_Init();
    MX_DAC_Init();
    MX_TIM2_Init();
    MX_TIM12_Init();
    MX_UART4_Init();
    MX_USART3_UART_Init();

    PLC_RS485_Init();

    osKernelInitialize();

    defaultTaskHandle = osThreadNew(state_machine, NULL, &defaultTask_attributes);
    ob30TaskHandle     = osThreadNew(OB30_Task, NULL, &ob30Task_attributes);
    commTaskHandle      = osThreadNew(CommTask, NULL, &commTask_attributes);

    osKernelStart();

    /* Should never reach here */
    while (1) { }
}

/* --- HAL callbacks --- */

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM2) {
        OB80_ISR_Handler();
    }
}

/* HAL_UART_RxCpltCallback for USART3 lives in comm_rs485.c */

void Error_Handler(void)
{
    __disable_irq();
    while (1) { }
}

/* --- printf() redirection to UART4, used by update_output()'s
 *     diagnostic line in process_image.c --- */
int _write(int file, char *ptr, int len)
{
    (void)file;
    HAL_UART_Transmit(&huart4, (uint8_t *)ptr, len, HAL_MAX_DELAY);
    return len;
}

/* ==================================================================
 * CubeMX-generated peripheral init (unchanged from original project -
 * copy the actual bodies from your .ioc-generated main.c here; these
 * are standard boilerplate and were not part of the architectural
 * cleanup). Stubs shown for structure only.
 * ================================================================== */

void SystemClock_Config(void)
{
    /* RCC_OscInitTypeDef / RCC_ClkInitTypeDef setup, PLL config, etc.
     * Copy verbatim from your original project. */
}

static void MX_GPIO_Init(void)
{
    /* GPIO clock enables + HAL_GPIO_Init() calls for every pin used
     * in io_config.c's din_map/dout_map/ain_map/aout_map, plus the
     * RS-485 DE pin (GPIOD11) and any pins used by ADC/DAC/TIM.
     *
     * IMPORTANT (carried over from analysis, not fixed here):
     * confirm GPIOC5/PC6 pin roles before wiring a real Start/Stop
     * input in plc_scheduler.c - GPIOC6 is currently configured as
     * dout1's output pin, which would conflict with using it as a
     * Start/Stop input. */
}

static void MX_ADC1_Init(void)
{
    /* ADC1 init matching ain_map[] channel assignments in io_config.c */
}

static void MX_DAC_Init(void)
{
    /* DAC init for channels 1-2 (aout1/aout2) */
}

static void MX_TIM2_Init(void)
{
    /* TIM2 init - used as the scan-overrun watchdog timer in
     * plc_scheduler.c's OB1(). Configure period so its interrupt
     * fires at MAX_SCAN_TIME_MS. */
}

static void MX_TIM12_Init(void)
{
    /* TIM12 PWM init for aout3/aout4 */
}

static void MX_UART4_Init(void)
{
    /* UART4 init - diagnostics output (see printf redirection above) */
}

static void MX_USART3_UART_Init(void)
{
    /* USART3 init - RS-485 transport (see comm_rs485.c) */
}
