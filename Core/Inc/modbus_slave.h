#ifndef MODBUS_SLAVE_H
#define MODBUS_SLAVE_H


#include <stdint.h>

#define MODBUS_SLAVE_ID  1

void Modbus_ProcessFrame(uint8_t *frame, uint16_t len);

#endif /* MODBUS_SLAVE_H */
