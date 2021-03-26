#include <inttypes.h>
#include <util/crc16.h>
#include <avr/io.h>
#include <util/twi.h>

#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#define BUFFER_SIZE 8

#define SLAVE_ADDRESS 0xA0
#define MASTER_ADDRESS 0xB0

typedef struct frame
{
	uint8_t control;
	uint8_t crc;
	uint8_t data[BUFFER_SIZE];
} FRAME;