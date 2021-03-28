#include "../shared.h"

void twi_slave(FRAME *frame)
{
	uint8_t byte = 0;
	uint8_t stream[STREAM_SIZE];
	bool link_established = true;

	// state: idle
	// action: select slave receiver mode
	TWCR = _BV(TWINT) | _BV(TWEA) | _BV(TWEN);

	// wait until action complete
	loop_until_bit_is_set(TWCR, TWINT);
	
	do
	{
		switch (TW_STATUS)
		{
		// state: received ping in write mode
		// action: send acknowledge
		case TW_SR_SLA_ACK:
			TWCR = _BV(TWINT) | _BV(TWEA) | _BV(TWEN);
			break;

		// state: received data and acknowledged
		// action: store data
		case TW_SR_DATA_ACK:
			TWCR = _BV(TWINT) | _BV(TWEA) | _BV(TWEN);
			stream[byte++] = TWDR;
			break;

		// state: transmission complete
		// action: end connection, check frame
		case TW_SR_STOP:
			memcpy(frame, stream, FRAME_SIZE);
			link_established = false;
			break;
		
		default:
			link_established = false;
			break;
		}

		// wait until action complete
		loop_until_bit_is_set(TWCR, TWINT);
	} while (link_established);
}

int main()
{
	__builtin_avr_cli();
	TWAR = SLAVE_ADDRESS;
	DDRD = 0xFF;

	FRAME my_frame;

	__builtin_avr_sei();

	while (1)
	{
		twi_slave(&my_frame);
		uint8_t received_crc = 0;
		for (int i = 0; i < BUFFER_SIZE; i++)
		{
			received_crc = _crc8_ccitt_update(received_crc, my_frame.data[i]);
		}

		if (received_crc == my_frame.crc)
		{
			PORTD = ~PORTD;
		}
	}

	return 0;
}