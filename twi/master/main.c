#include "../shared.h"

void twi_master_receiever(FRAME *frame, const uint8_t address)
{
	bool link_established = true;
	uint8_t byte = 0;
	uint8_t stream[STREAM_SIZE];

	// state: idle
	// action: send start condition
	TWCR = _BV(TWINT) | _BV(TWSTA) | _BV(TWEN);
	loop_until_bit_is_set(TWCR, TWINT);

	do
	{
		switch (TW_STATUS)
		{
		// state: start condition sent
		// action: send SLA+W/R
		case TW_START:
			TWDR = __builtin_avr_insert_bits(0xfffffff0, TW_READ, address);
			TWCR = _BV(TWINT) | _BV(TWEN);
			break;

		case TW_MR_DATA_ACK:
			stream[byte++] = TWDR;
		case TW_MR_SLA_ACK:
			TWCR = _BV(TWINT) | _BV(TWEN) | _BV(TWEA);
			break;

		// state: missed acknowledge window
		// action: end transmission
		default:
			link_established = false;
			break;
		}

		loop_until_bit_is_set(TWCR, TWINT);
	} while ((byte < FRAME_SIZE) && link_established);

	// state: transmission complete
	// action: send stop condition
	TWCR = _BV(TWINT) | _BV(TWSTO) | _BV(TWEN);

	memcpy(frame, stream, FRAME_SIZE);
}

void twi_master_transmitter(const FRAME *frame, const uint8_t address)
{
	bool link_established = true;

	uint8_t byte = 0;
	uint8_t stream[STREAM_SIZE];

	// state: idle
	// action: send start condition
	TWCR = _BV(TWINT) | _BV(TWSTA) | _BV(TWEN);
	loop_until_bit_is_set(TWCR, TWINT);

	do
	{
		switch (TW_STATUS)
		{
		// state: start condition sent
		// action: send SLA+W/R
		case TW_START:
			TWDR = __builtin_avr_insert_bits(0xfffffff0, TW_WRITE, address);
			TWCR = _BV(TWINT) | _BV(TWEN);
			break;

		// state: slave acknowledged ping
		// action: prepare frame and send first byte
		case TW_MT_SLA_ACK:
			memcpy(stream, frame, FRAME_SIZE);
		// state: slave acknowledged byte
		// action: send next byte
		case TW_MT_DATA_ACK:
			TWDR = stream[byte++];
			TWCR = _BV(TWINT) | _BV(TWEN);
			break;

		// state: missed acknowledge window
		// action: end transmission
		default:
			link_established = false;
			break;
		}

		loop_until_bit_is_set(TWCR, TWINT);
	} while ((byte < FRAME_SIZE) && link_established);

	// state: transmission complete
	// action: send stop condition
	TWCR = _BV(TWINT) | _BV(TWSTO) | _BV(TWEN);
}
void init_frame(FRAME *frame)
{
	frame->control = 0xFF;

	frame->crc = 0;
	for (size_t i = 0; i < BUFFER_SIZE; ++i)
	{
		frame->data[i] = 'a' + i;
		frame->crc = _crc8_ccitt_update(frame->crc, frame->data[i]);
	}	
}
int main()
{
	__builtin_avr_cli();
	DDRD = 0xFF;

	TWBR = 0xFF;
	TWAR = MASTER_ADDRESS;

	__builtin_avr_sei();

	while (1)
	{
		FRAME my_frame;
		twi_master_receiever(&my_frame, SLAVE_ADDRESS);

		uint8_t received_crc = 0;
		for (int i = 0; i < BUFFER_SIZE; i++)
		{
			received_crc = _crc8_ccitt_update(received_crc, my_frame.data[i]);
		}

		if (received_crc == my_frame.crc)
		{
			PORTD = ~PORTD;
		}

		__builtin_avr_delay_cycles(F_CPU);
	}

	return 0;
}