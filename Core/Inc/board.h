#ifndef BOARD_H
#define BOARD_H
/*
 * board.h
 * -------
 * Shared HAL peripheral handles. Defined once in main.c, declared
 * extern everywhere else that needs them. This replaces the previous
 * pattern of every file re-declaring "ADC_HandleTypeDef hadc1;" etc,
 * which is what made final_plc.c / final_plc.h impossible to both
 * #include safely (duplicate definitions).
 */

#include "main.h"

extern ADC_HandleTypeDef hadc1;
extern DAC_HandleTypeDef hdac;
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim12;
extern UART_HandleTypeDef huart4;   /* diagnostics / printf */
extern UART_HandleTypeDef huart3;   /* RS-485 */

#endif /* BOARD_H */
