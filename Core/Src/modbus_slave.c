#include "modbus_slave.h"
#include "comm_rs485.h"
#include "process_image.h"
#include <string.h>

#define FC_READ_COILS            0x01
#define FC_READ_DISCRETE_INPUTS  0x02
#define FC_READ_HOLDING_REGS     0x03
#define FC_READ_INPUT_REGS       0x04
#define FC_WRITE_SINGLE_COIL     0x05
#define FC_WRITE_SINGLE_REG      0x06
#define FC_WRITE_MULTIPLE_COILS  0x0F
#define FC_WRITE_MULTIPLE_REGS   0x10

#define EXC_ILLEGAL_FUNCTION     0x01
#define EXC_ILLEGAL_ADDRESS      0x02
#define EXC_ILLEGAL_VALUE        0x03

static uint8_t tx_buf[RS485_BUF_SIZE];

static uint16_t modbus_crc16(const uint8_t *buf, uint16_t len)
{
    uint16_t crc = 0xFFFF;
    for (uint16_t pos = 0; pos < len; pos++) {
        crc ^= (uint16_t)buf[pos];
        for (int i = 0; i < 8; i++) {
            if (crc & 0x0001) {
                crc >>= 1;
                crc ^= 0xA001;
            } else {
                crc >>= 1;
            }
        }
    }
    return crc;
}

static void send_response(uint16_t len)
{
    uint16_t crc = modbus_crc16(tx_buf, len);
    tx_buf[len]     = (uint8_t)(crc & 0xFF);
    tx_buf[len + 1] = (uint8_t)((crc >> 8) & 0xFF);
    PLC_RS485_Transmit(tx_buf, len + 2);
}

static void send_exception(uint8_t function, uint8_t exception_code)
{
    tx_buf[0] = MODBUS_SLAVE_ID;
    tx_buf[1] = function | 0x80;
    tx_buf[2] = exception_code;
    send_response(3);
}

void Modbus_ProcessFrame(uint8_t *frame, uint16_t len)
{
    if (len < 4) return;                      /* too short to be valid */
    if (frame[0] != MODBUS_SLAVE_ID) return;   /* not addressed to us */

    uint16_t recv_crc = (uint16_t)frame[len - 2] | ((uint16_t)frame[len - 1] << 8);
    if (modbus_crc16(frame, len - 2) != recv_crc) return; /* bad CRC: no reply, per spec */

    uint8_t function = frame[1];

    switch (function) {

    case FC_READ_DISCRETE_INPUTS: {
        uint16_t start = ((uint16_t)frame[2] << 8) | frame[3];
        uint16_t qty   = ((uint16_t)frame[4] << 8) | frame[5];
        if (qty == 0 || start + qty > NUM_DIN) { send_exception(function, EXC_ILLEGAL_ADDRESS); break; }

        uint8_t byte_count = (qty + 7) / 8;
        tx_buf[0] = MODBUS_SLAVE_ID;
        tx_buf[1] = function;
        tx_buf[2] = byte_count;
        memset(&tx_buf[3], 0, byte_count);
        for (uint16_t i = 0; i < qty; i++) {
            if (DI_table[start + i]) tx_buf[3 + (i / 8)] |= (1 << (i % 8));
        }
        send_response(3 + byte_count);
        break;
    }

    case FC_READ_COILS: {
        uint16_t start = ((uint16_t)frame[2] << 8) | frame[3];
        uint16_t qty   = ((uint16_t)frame[4] << 8) | frame[5];
        if (qty == 0 || start + qty > NUM_DOUT) { send_exception(function, EXC_ILLEGAL_ADDRESS); break; }

        uint8_t byte_count = (qty + 7) / 8;
        tx_buf[0] = MODBUS_SLAVE_ID;
        tx_buf[1] = function;
        tx_buf[2] = byte_count;
        memset(&tx_buf[3], 0, byte_count);
        for (uint16_t i = 0; i < qty; i++) {
            if (DQ_table[start + i]) tx_buf[3 + (i / 8)] |= (1 << (i % 8));
        }
        send_response(3 + byte_count);
        break;
    }

    case FC_READ_INPUT_REGS: {
        uint16_t start = ((uint16_t)frame[2] << 8) | frame[3];
        uint16_t qty   = ((uint16_t)frame[4] << 8) | frame[5];
        if (qty == 0 || start + qty > NUM_AIN) { send_exception(function, EXC_ILLEGAL_ADDRESS); break; }

        tx_buf[0] = MODBUS_SLAVE_ID;
        tx_buf[1] = function;
        tx_buf[2] = (uint8_t)(qty * 2);
        for (uint16_t i = 0; i < qty; i++) {
            tx_buf[3 + i * 2]     = (uint8_t)(AI_table[start + i] >> 8);
            tx_buf[3 + i * 2 + 1] = (uint8_t)(AI_table[start + i] & 0xFF);
        }
        send_response(3 + qty * 2);
        break;
    }

    case FC_READ_HOLDING_REGS: {
        uint16_t start = ((uint16_t)frame[2] << 8) | frame[3];
        uint16_t qty   = ((uint16_t)frame[4] << 8) | frame[5];
        if (qty == 0 || start + qty > NUM_AOUT) { send_exception(function, EXC_ILLEGAL_ADDRESS); break; }

        tx_buf[0] = MODBUS_SLAVE_ID;
        tx_buf[1] = function;
        tx_buf[2] = (uint8_t)(qty * 2);
        for (uint16_t i = 0; i < qty; i++) {
            tx_buf[3 + i * 2]     = (uint8_t)(AQ_table[start + i] >> 8);
            tx_buf[3 + i * 2 + 1] = (uint8_t)(AQ_table[start + i] & 0xFF);
        }
        send_response(3 + qty * 2);
        break;
    }

    case FC_WRITE_SINGLE_COIL: {
        uint16_t addr = ((uint16_t)frame[2] << 8) | frame[3];
        uint16_t val  = ((uint16_t)frame[4] << 8) | frame[5];
        if (addr >= NUM_DOUT) { send_exception(function, EXC_ILLEGAL_ADDRESS); break; }
        if (val != 0x0000 && val != 0xFF00) { send_exception(function, EXC_ILLEGAL_VALUE); break; }

        DQ_table[addr] = (val == 0xFF00) ? 1 : 0;

        memcpy(tx_buf, frame, 6);  /* echo request per Modbus spec */
        send_response(6);
        break;
    }

    case FC_WRITE_SINGLE_REG: {
        uint16_t addr = ((uint16_t)frame[2] << 8) | frame[3];
        uint16_t val  = ((uint16_t)frame[4] << 8) | frame[5];
        if (addr >= NUM_AOUT) { send_exception(function, EXC_ILLEGAL_ADDRESS); break; }
        if (val > 4095) { send_exception(function, EXC_ILLEGAL_VALUE); break; }

        AQ_table[addr] = val;

        memcpy(tx_buf, frame, 6);  /* echo request per Modbus spec */
        send_response(6);
        break;
    }

    case FC_WRITE_MULTIPLE_COILS: {
        uint16_t start = ((uint16_t)frame[2] << 8) | frame[3];
        uint16_t qty   = ((uint16_t)frame[4] << 8) | frame[5];
        uint8_t byte_count = frame[6];
        if (qty == 0 || start + qty > NUM_DOUT) { send_exception(function, EXC_ILLEGAL_ADDRESS); break; }
        if (byte_count != (qty + 7) / 8)        { send_exception(function, EXC_ILLEGAL_VALUE);   break; }

        for (uint16_t i = 0; i < qty; i++) {
            uint8_t bit = (frame[7 + (i / 8)] >> (i % 8)) & 0x01;
            DQ_table[start + i] = bit;
        }

        tx_buf[0] = MODBUS_SLAVE_ID;
        tx_buf[1] = function;
        tx_buf[2] = frame[2]; tx_buf[3] = frame[3];
        tx_buf[4] = frame[4]; tx_buf[5] = frame[5];
        send_response(6);
        break;
    }

    case FC_WRITE_MULTIPLE_REGS: {
        uint16_t start = ((uint16_t)frame[2] << 8) | frame[3];
        uint16_t qty   = ((uint16_t)frame[4] << 8) | frame[5];
        uint8_t byte_count = frame[6];
        if (qty == 0 || start + qty > NUM_AOUT) { send_exception(function, EXC_ILLEGAL_ADDRESS); break; }
        if (byte_count != qty * 2)              { send_exception(function, EXC_ILLEGAL_VALUE);   break; }

        for (uint16_t i = 0; i < qty; i++) {
            uint16_t val = ((uint16_t)frame[7 + i * 2] << 8) | frame[7 + i * 2 + 1];
            if (val > 4095) { send_exception(function, EXC_ILLEGAL_VALUE); return; }
            AQ_table[start + i] = val;
        }

        tx_buf[0] = MODBUS_SLAVE_ID;
        tx_buf[1] = function;
        tx_buf[2] = frame[2]; tx_buf[3] = frame[3];
        tx_buf[4] = frame[4]; tx_buf[5] = frame[5];
        send_response(6);
        break;
    }

    default:
        send_exception(function, EXC_ILLEGAL_FUNCTION);
        break;
    }
}
