#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <stdint.h>

FUSES = {
    .low = FUSE_CKSEL0,
    .high = FUSE_SPIEN & FUSE_BODLEVEL2 & FUSE_BODLEVEL1,
    .extended = EFUSE_DEFAULT};

#define ENC_COUNT 2
#define READ_MASK 0x11
#define ENC_CL 0x93
#define ENC_ACL 0x39

// pins A and B of the encoder {{A, B}, ...}
const uint8_t ENC_PINS[ENC_COUNT][2] PROGMEM = {{_BV(PC1), _BV(PC0)}, {_BV(PC3), _BV(PC2)}};
volatile int value[ENC_COUNT];

ISR(PCINT1_vect)
{
    static uint8_t pattern[ENC_COUNT];

    for (uint8_t n = 0; n < ENC_COUNT; ++n)
    {
        uint8_t a = ((PINC & pgm_read_byte(&ENC_PINS[n][0])) != 0) << 4;
        uint8_t b = (PINC & pgm_read_byte(&ENC_PINS[n][1])) != 0;
        uint8_t signal = a | b;

        if (signal ^ (pattern[n] & READ_MASK))
        {
            // construct bit pattern of changed encoder
            pattern[n] = __builtin_avr_insert_bits(0x654f210f, pattern[n], signal);

            if (pattern[n] == ENC_CL)
                ++value[n];
            else if (pattern[n] == ENC_ACL)
                --value[n];
        }
    }
}

int main(void)
{
    cli();                                                            // disable interrupts for setup
    DDRC &= ~(_BV(PC0) | _BV(PC1) | _BV(PC2) | _BV(PC3));             // set as input
    PORTC |= _BV(PC0) | _BV(PC1) | _BV(PC2) | _BV(PC3);               // enable pullup
    PCICR = _BV(PCIE1);                                               // enable interrups on pin change
    PCMSK1 = _BV(PCINT8) | _BV(PCINT9) | _BV(PCINT10) | _BV(PCINT11); // set interrupt masks
    DDRD = 0xFF;                                                      // set all port d as output
    sei();                                                            // enable interrupts globally

    while (1)
    {
        PORTD = value[0] & 0xFF;
        continue;
    }

    return 0;
}