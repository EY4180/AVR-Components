#include "../shared.h"

#define FRAME_SIZE (sizeof(FRAME))

FUSES = {
	.low = FUSE_CKSEL0,
	.high = FUSE_SPIEN & FUSE_BODLEVEL2 & FUSE_BODLEVEL1,
	.extended = EFUSE_DEFAULT};

void twi_master_transmitter(FRAME *frame, uint8_t address)
{
	uint8_t transmitted_bytes = 0;
	uint8_t stream[FRAME_SIZE];
	memcpy(stream, frame, FRAME_SIZE);

	// state: idle
	// action: send start condition
	TWCR = _BV(TWINT) | _BV(TWSTA) | _BV(TWEN);

	while (transmitted_bytes < FRAME_SIZE)
	{
		switch (TW_STATUS)
		{
		// state: start condition sent
		// action: send SLA+W
		case TW_START:
			TWDR = address;
			TWCR = _BV(TWINT) | _BV(TWEN);
			break;

		// state: slave acknowledged ping
		// action: send first byte
		case TW_MT_SLA_ACK:

		// state: slave acknowledged byte
		// action: send next byte
		case TW_MT_DATA_ACK:
			TWDR = stream[transmitted_bytes++];
			TWCR = _BV(TWINT) | _BV(TWEN);
			break;

		// state: missed acknowledge window
		// action: end transmission
		case TW_MT_DATA_NACK:
		case TW_MT_SLA_NACK:
			goto BUS_ERROR;

		default:
			break;
		}

		// wait until action complete
		loop_until_bit_is_set(TWCR, TWINT);
	}

BUS_ERROR:

	// state: transmission complete
	// action: send stop condition
	TWCR = _BV(TWINT) | _BV(TWSTO) | _BV(TWEN);
}

int main()
{
	__builtin_avr_cli();
	TWBR = 0xFF;
	TWAR = MASTER_ADDRESS;

	FRAME my_frame;
	my_frame.crc = 0;
	my_frame.control = 0xFF;

	for (uint8_t i = 0; i < BUFFER_SIZE; i++)
	{
		my_frame.data[i] = 'a' + i;
		my_frame.crc = _crc8_ccitt_update(my_frame.crc, my_frame.data[i]);
	}

	__builtin_avr_sei();

	while (1)
	{
		twi_master_transmitter(&my_frame, SLAVE_ADDRESS);
		__builtin_avr_delay_cycles(F_CPU);
	}

	return 0;
}