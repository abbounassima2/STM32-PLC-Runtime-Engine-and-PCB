#ifndef COMM_RS485_H
#define COMM_RS485_H
/*
 * comm_rs485.h
 * ------------
 * RS-485 transport: interrupt-driven byte capture + inter-frame-gap
 * timeout framing. This is transport only - NO protocol (Modbus RTU
 * etc.) is parsed yet; CommTask() just detects "a frame arrived" and
 * discards it. See analysis section on the Communication Manager for
 * where a real protocol parser plugs in.
 *
 * NOTE: the original code also had orphaned declarations for
 * RS485_Init / RS485_Transmit / RS485_CheckTimeout (different names
 * from the PLC_RS485_* functions actually used) with no definitions
 * anywhere. Those were dead code and have been removed here - only
 * the functions that are actually implemented and called are kept.
 */

#include <stdint.h>
#include "plc_config.h"

extern uint8_t rs485_rx_buffer[RS485_BUF_SIZE];
extern volatile uint16_t rs485_rx_index;
extern volatile uint8_t rs485_frame_ready;

void PLC_RS485_Init(void);
void PLC_RS485_Transmit(uint8_t *data_buffer, uint16_t data_size);
void PLC_RS485_CheckTimeout(void);

/* RTOS task: polls the timeout + hands off completed frames.
 * TODO: replace the "discard and reset" body with an actual Modbus
 * RTU frame parser once the protocol layer is implemented. */
void CommTask(void *argument);

#endif /* COMM_RS485_H */
