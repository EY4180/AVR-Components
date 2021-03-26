#include "../shared.h"



FUSES = {
	.low = FUSE_CKSEL0,
	.high = FUSE_SPIEN & FUSE_BODLEVEL2 & FUSE_BODLEVEL1,
	.extended = EFUSE_DEFAULT};

void twi_slave(FRAME *frame)
{
	uint8_t data_index = 0;

	// state: idle
	// action: select slave receiver mode
	TWCR = _BV(TWEA) | _BV(TWEN);

	while (data_index < BUFFER_SIZE)
	{
		switch (TW_STATUS)
		{
		// state: received ping
		// action: send acknowledge
		case TW_SR_SLA_ACK:

			break;

		// state: received data and acknowledged
		// action: store data
		case TW_SR_DATA_ACK:
			
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
	TWAR = SLAVE_ADDRESS;

	__builtin_avr_sei();
	/* code */
	return 0;
}
