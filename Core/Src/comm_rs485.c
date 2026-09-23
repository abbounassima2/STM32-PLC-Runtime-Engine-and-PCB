#include "comm_rs485.h"
#include "board.h"
#include "cmsis_os.h"

uint8_t rs485_rx_buffer[RS485_BUF_SIZE];
volatile uint16_t rs485_rx_index = 0;
volatile uint8_t rs485_frame_ready = 0;

static uint32_t last_rx_time = 0;
static uint8_t temp_byte = 0;

void PLC_RS485_Init(void)
{
    rs485_rx_index = 0;
    rs485_frame_ready = 0;
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_11, GPIO_PIN_RESET); /* DE pin: listen mode */
    HAL_UART_Receive_IT(&huart3, &temp_byte, 1);
}

void PLC_RS485_Transmit(uint8_t *data_buffer, uint16_t data_size)
{
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_11, GPIO_PIN_SET);
    HAL_UART_Transmit(&huart3, data_buffer, data_size, HAL_MAX_DELAY);
    while (__HAL_UART_GET_FLAG(&huart3, UART_FLAG_TC) == RESET);
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_11, GPIO_PIN_RESET);
}

/* Kept as a global HAL callback (HAL dispatches by symbol name) */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART3) {
        if (rs485_frame_ready == 0) {
            if (rs485_rx_index < RS485_BUF_SIZE) {
                rs485_rx_buffer[rs485_rx_index++] = temp_byte;
                last_rx_time = HAL_GetTick();
            }
        }
        HAL_UART_Receive_IT(&huart3, &temp_byte, 1);
    }
}

void PLC_RS485_CheckTimeout(void)
{
    if (rs485_rx_index > 0 && rs485_frame_ready == 0) {
        if ((HAL_GetTick() - last_rx_time) > RS485_FRAME_TIMEOUT_MS) {
            rs485_frame_ready = 1;
        }
    }
}

void CommTask(void *argument)
{
    (void)argument;
    for (;;) {
        PLC_RS485_CheckTimeout();

        if (rs485_frame_ready) {
            /* TODO: Modbus_ProcessFrame(rs485_rx_buffer, rs485_rx_index); */
            rs485_rx_index = 0;
            rs485_frame_ready = 0;
        }

        osDelay(10);
    }
}
