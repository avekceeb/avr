// 12MHz quarz
#define F_CPU 12000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

/*
 * PB0 ---> a
 * PB1 ---> b
 * PB2 ---> c
 * PB3 ---> d
 * PB4 ---> e
 * PB5 ---> f
 * PB6 ---> g
 * PB7 ---> h 
 * PD0
 * PD1
 * PD2 <--- counter:divider0 (INT0) 
 * PD3 <--- counter:divider1 (INT1)
 * PD4
 * PD5
 * PD6
 */

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

// max 65535
volatile uint16_t counter = 0;     

#define divider0 100
#define divider1 1000

ISR(INT0_vect) {
    counter++;
    // set led as counter 1:divider0
    PORTB = pgm_read_byte(&(
                segments[(counter/divider0) & 0x0f]
                ));
    // mark that divider is '0' 
    PORTB &= ~(_BV(PB7));
}

ISR(INT1_vect) {
    counter++;
    // set led as counter 1:divider1
    PORTB = pgm_read_byte(&(
                segments[(counter/divider1) & 0x0f]
                ));
    // mark that divider is '1' 
    PORTB |= _BV(PB7);
}

ISR(TIMER1_OVF_vect) {
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
    // timer to 1 Hz:
    // 12000000.0 / (2*1024*(5858 + 1)) = 1.000064004096262
    // prescale N = 1024
    // OCR1A = 5858
    // enable timer overflow interrupt for both Timer0 and Timer1
    TIMSK = _BV(TOIE1);
    // set timer0 counter initial value to 0
    TCNT1 = 0x00;
    // start timer0 with /1024 prescaler
    // TCCR0 = (1<<CS02) | (1<<CS00);
    // lets turn on 16 bit timer1 also with /1024
    // TCCR1B |= (1 << CS10) | (1 << CS12);
    sei();    

#if 0
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

