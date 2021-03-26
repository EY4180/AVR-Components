#include "../shared.h"

#define FRAME_SIZE sizeof(FRAME)

FUSES = {
	.low = FUSE_CKSEL0,
	.high = FUSE_SPIEN & FUSE_BODLEVEL2 & FUSE_BODLEVEL1,
	.extended = EFUSE_DEFAULT};

void twi_slave_receiver(FRAME *frame)
{
	uint8_t received_bytes = 0;
	uint8_t stream[FRAME_SIZE];
	bool listening = true;

	// state: idle
	// action: select slave receiver mode
	TWCR = _BV(TWINT) | _BV(TWEA) | _BV(TWEN);
	
	while (listening)
	{
		switch (TW_STATUS)
		{
		// state: received ping
		// action: send acknowledge
		case TW_SR_SLA_ACK:
			TWCR = _BV(TWINT) | _BV(TWEA) | _BV(TWEN);
			break;

		// state: received data and acknowledged
		// action: store data
		case TW_SR_DATA_ACK:
			TWCR = _BV(TWINT) | _BV(TWEA) | _BV(TWEN);
			stream[received_bytes++] = TWDR;
			break;

		case TW_SR_STOP:
			memcpy(frame, stream, FRAME_SIZE);
			listening = false;
			break;

		default:
			break;
		}

		// wait until action complete
		loop_until_bit_is_set(TWCR, TWINT);
	}
}

int main()
{
	__builtin_avr_cli();
	FRAME my_frame;
	TWAR = SLAVE_ADDRESS;
	DDRD = 0xFF;

	__builtin_avr_sei();

	while (1)
	{
		twi_slave_receiver(&my_frame);
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
	
	/* code */
	return 0;
}
