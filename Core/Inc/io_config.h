#ifndef IO_CONFIG_H
#define IO_CONFIG_H




#include "main.h"
#include "plc_config.h"
#include <stdint.h>


typedef enum { TYPE_DIN ,TYPE_DOUT ,TYPE_AIN , TYPE_AOUT } ChannelType_t ;


typedef struct {
	ChannelType_t type;
	GPIO_TypeDef *port; /*digital only*/
	uint16_t pin; /*digital only*/
	uint8_t channel ;
	uint8_t index;
	const char *tag
}PinMap_t;


extern PinMap_t din_map[NUM_DIN];
extern PinMap_t dout_map[NUM_DOUT];
extern PinMap_t ain_map[NUM_AIN];
extern PinMap_t aout_map[NUM_AOUT];


#endif
