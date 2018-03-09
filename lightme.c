
#define F_CPU 12000000UL

#include <avr/io.h>
#include <avr/interrupt.h>

/*
 * wait for signal and turn on the light
 */

// turn led
ISR(INT0_vect) {
    PORTB |= (_BV(PB0));
}

// clear led
ISR(INT1_vect) {
    PORTB &= ~(_BV(PB0));
}

int main(void) {

    DDRD = 0;
    PORTD |= (_BV(PD2) | _BV(PD3));

    DDRB |= (_BV(PB0) | _BV(PB3)); // PB3 = OCR1A (for tiny2313) ; PB1 (for mega8)

    // 1 Hz
    OCR1A = 5859;
    TCNT1 = 0;

    // CS12 | CS10 : prescaler 1024
    // WGM12 : mode CTC ; top in OCR1A ;
    //         update OCR1x Immediate ; TOV1 flag set on MAX
    TCCR1B |= (_BV(WGM12) | _BV(CS12) | _BV(CS10)) ;
#if 1
    // toggle OC1A on compare match
    // none interrupt needed
    TCCR1A = _BV(COM1A0);
#endif

    // enable interrupts int0 int1 (PD2 PD3)
    GIMSK = _BV(INT0) | _BV(INT1);
    // into and int1 by rising edges
    MCUCR = _BV(ISC11) | _BV(ISC10) | _BV(ISC01) | _BV(ISC00);

    sei();

    while (1) {
    }
}

