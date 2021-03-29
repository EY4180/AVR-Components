#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define seconds_to_cycles(time, scale) ((time * F_CPU) / scale)

#define REGISTER_SELECT _BV(PB1)
#define ENABLE _BV(PB0)

#define BUFFER_SIZE 16

#define LINE_WIDTH 8

#define CLEAR_DISPLAY 0x01
#define RETURN_HOME 0x02
#define SET_ENTRY_MODE 0x06
#define DISPLAY_ON 0x0C
#define DISPLAY_OFF 0x0B
#define FUNCTION_SET 0x38
#define FUNCTION_RESET 0x30
#define SET_DDRAM_ADDRESS 0x80
#define LINE_ADDRESS_0 0x00
#define LINE_ADDRESS_1 0x40

#define STATUS GPIOR0
#define UPDATE_DISPLAY 0

volatile unsigned update_count;

void write_data(const uint8_t data, bool data_byte)
{
	PORTB |= ENABLE;

	if (data_byte)
	{
		PORTB |= REGISTER_SELECT;
	}
	else
	{
		PORTB &= ~REGISTER_SELECT;
	}

	PORTD = data;

	_delay_us(100);
	PORTB &= ~ENABLE;
	_delay_us(100);
}

void write_string(const char *str, const int length, const uint8_t line)
{
	write_data(SET_DDRAM_ADDRESS | line, false);

	for (size_t i = 0; i < length; ++i)
	{
		write_data(str[i], true);
	}
}

void init_lcd()
{
	const uint8_t instruction[] = {
		FUNCTION_RESET,
		FUNCTION_RESET,
		FUNCTION_RESET,
		FUNCTION_SET,
		DISPLAY_OFF,
		CLEAR_DISPLAY,
		SET_ENTRY_MODE,
		DISPLAY_ON};

	const int length = sizeof(instruction) / sizeof(*instruction);

	for (size_t i = 0; i < length; ++i)
	{
		_delay_ms(200);
		write_data(instruction[i], false);
	}
}

int main()
{
	__builtin_avr_cli();
	// setup port directions
	DDRD = 0xff;
	DDRB = ENABLE | REGISTER_SELECT;

	TCCR1A = 0x00;
	TCCR1B = _BV(CS12);
	OCR1A = seconds_to_cycles(0.5, 256);
	TIMSK1 = _BV(OCIE1A);

	init_lcd();
	__builtin_avr_sei();

	while (1)
	{
		if (bit_is_set(STATUS, UPDATE_DISPLAY))
		{
			char buffer[BUFFER_SIZE];
			itoa(update_count, buffer, 10);
			write_string(buffer, strlen(buffer), LINE_ADDRESS_0);

			STATUS &= ~_BV(UPDATE_DISPLAY);
		}
	}

	return 0;
}

ISR(TIMER1_COMPA_vect)
{
	++update_count;
	STATUS |= _BV(UPDATE_DISPLAY);
}