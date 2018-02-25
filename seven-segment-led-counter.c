// 12MHz quarz
#define F_CPU 12000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>


/*
       a   
    +-----+
   f|     |b
    |  g  |
    +-----+
   e|     |c
    |     |
    +-----+   * h
       d
*/

static const unsigned char segments[16] PROGMEM = {
	/* hgfedcba   */
	/* 00111111 0 */ 0x3f,
	/* 00000110 1 */ 0x06,
	/* 01011011 2 */ 0x5b,
	/* 01001111 3 */ 0x4f,
	/* 01100110 4 */ 0x66,
	/* 01101101 5 */ 0x6d,
	/* 01111101 6 */ 0x7d,
	/* 00000111 7 */ 0x07,
	/* 01111111 8 */ 0x7f,
	/* 01101111 9 */ 0x6f,
	/* 01011111 a */ 0x5f, 
	/* 01111100 b */ 0x7c, 
	/* 00111001 c */ 0x39, 
	/* 01011110 d */ 0x5e,
	/* 01111001 e */ 0x79,
	/* 01110001 f */ 0x71
};

volatile int counter = 0;     

ISR(INT0_vect) {
    counter++;
    // set led as counter 1:100
    PORTB = pgm_read_byte(&(
                segments[(counter/100) & 0x0f]
                ));
    // blink by 'dot'
    PORTB ^= _BV(PB7);
}

ISR(INT1_vect) {
    counter++;
    // set led as counter 1:1000
    PORTB = pgm_read_byte(&(
                segments[(counter/1000) & 0x0f]
                ));
    // blink by 'dot'
    PORTB ^= _BV(PB7);
}

int main() {
    // set all PB as output
    DDRB = 0xff;
    // all PD as in:
    DDRD = 0x00;

    // enable interrupts int0 int1 (PD2 PD3)
    GIMSK = _BV(INT0) | _BV(INT1);
    // into and int1 by rising edges
    MCUCR = _BV(ISC11) | _BV(ISC10) | _BV(ISC01) | _BV(ISC00);
    
    sei();    

#if 1
    for(int i=0; i<16; i++) {
        // check: show all numbers
        PORTB = pgm_read_byte(&(segments[i] ));
        _delay_ms(1000);
    }
#endif

    while (1) {
        {asm("nop");};
    } 
    return 0;
}

