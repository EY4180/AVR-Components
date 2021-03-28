#include "../shared.h"

void twi_master_transmitter(FRAME *frame, uint8_t address, uint8_t rw)
{
	bool sending = true;
	uint8_t transmitted_bytes = 0;
	uint8_t stream[FRAME_SIZE];
	memcpy(stream, frame, FRAME_SIZE);

	// state: idle
	// action: send start condition
	TWCR = _BV(TWINT) | _BV(TWSTA) | _BV(TWEN);

	// wait until action complete
	loop_until_bit_is_set(TWCR, TWINT);

	while (sending)
	{
		switch (TW_STATUS)
		{
		// state: start condition sent
		// action: send SLA+W/R
		case TW_START:
			TWDR = __builtin_avr_insert_bits(0xfffffff0, rw, address);
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
			sending = transmitted_bytes < FRAME_SIZE;
			break;

		// state: missed acknowledge window
		// action: end transmission
		case TW_MT_DATA_NACK:
		case TW_MT_SLA_NACK:
			sending = false;
			break;

		default:
			break;
		}

		// wait until action complete
		loop_until_bit_is_set(TWCR, TWINT);
	}

	// state: transmission complete
	// action: send stop condition
	TWCR = _BV(TWINT) | _BV(TWSTO) | _BV(TWEN);
}

int main()
{
	__builtin_avr_cli();
	TWBR = BAUD_REGISTER;
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
		twi_master_transmitter(&my_frame, SLAVE_ADDRESS, TW_WRITE);
		__builtin_avr_delay_cycles(F_CPU);
	}

	return 0;
}