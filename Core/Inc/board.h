#ifndef BOARD_H
#define BOARD_H

#include "main.h"

extern ADC_HandleTypeDef hadc1;
extern DAC_HandleTypeDef hdac;
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim12;
extern UART_HandleTypeDef huart4;   /* diagnostics / printf */
extern UART_HandleTypeDef huart3;   /* RS-485 */

#endif /* BOARD_H */
