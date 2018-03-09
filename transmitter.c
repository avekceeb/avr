
#define F_CPU 12000000UL

#include <avr/io.h>
#include <avr/interrupt.h>

/*
 * atmega8 Timer1 CTC mode
 *
 * f0=12000000.0 - external quarz frequency
 * f0/(2*(1+x))  - timer frequency
 * NO PRESCALER!
 * x - OCR1A
 *
 * x=186
 * 32085.5614973262
 * 
 * x=166
 * 35928.14371257485
 *
 * x=152
 * 39215.686274509804
 * 
 * x=149
 * 40000.0
 *
 * 0.5Hz ~= 12000000.0 / (2 * 1024 * 11719)
 * 2Hz   ~= 12000000.0 / (2 * 1024 * 2929)

 */

volatile char prescaler_id = 0;
volatile char counter_id = 0;

#define prescalers 5
#define counters 4

// WGM12 : mode CTC ; top in OCR1A ;
//         update OCR1x Immediate ; TOV1 flag set on MAX
#define no_prescaler (_BV(WGM12) | _BV(CS10))
#define prescaler_8 (_BV(WGM12) | _BV(CS11))
#define prescaler_64 (_BV(WGM12) | _BV(CS11) | _BV(CS10))
#define prescaler_256 (_BV(WGM12) | _BV(CS12))
#define prescaler_1024 (_BV(WGM12) | _BV(CS12) | _BV(CS10))

#define use_hw 1
#define use_timer_int 0

#if use_timer_int
ISR (TIMER1_COMPA_vect) {
    PORTB ^= (_BV(PB0));
}
#endif

// go through counters
// pressing button connected to INT0
// will change frequency in circle:
//  +-> 32 -> 36 -> 39.2 -> 40kHz ->+
//  ^                               |
//  +-------------------------------+
ISR(INT0_vect) {
    if (counter_id++ >= counters) {
        counter_id = 0;
    }
    switch (counter_id) {
        // 32kHz
        case 0: OCR1A = 186; break;
        case 1: OCR1A = 166; break;
        case 2: OCR1A = 152; break;
        // 40kHz
        case 3: OCR1A = 149; break;
    }
    TCNT1 = 0;
}

// go through prescalers
// button to INT1 will change
// prescaler : no -> 8 -> 63 -> 256 -> 1024 -> ..
ISR(INT1_vect) {
    if (prescaler_id++ >= prescalers) {
        prescaler_id = 0;
    }
    switch (prescaler_id) {
        case 0: TCCR1B = no_prescaler ; break;
        case 1: TCCR1B = prescaler_8 ; break;
        case 2: TCCR1B = prescaler_64 ; break;
        case 3: TCCR1B = prescaler_256 ; break;
        case 4: TCCR1B = prescaler_1024 ; break;
    }
    TCNT1 = 0;
}

int main(void) {

    DDRD = 0;
    PORTD |= (_BV(PD2) | _BV(PD3));

    DDRB |= (_BV(PB1));

#if use_timer_int
    DDRB |= (_BV(PB0)); 
#endif

    OCR1A = 186;
    TCNT1 = 0;

    TCCR1B = no_prescaler ;

#if use_hw
    // toggle OC1A on compare match
    // no interrupt needed
    TCCR1A = _BV(COM1A0);
#endif

#if use_timer_int
    // interrupt on compare match
    TIMSK |= (_BV(OCIE1A));
#endif

    // enable interrupts int0 int1 (PD2 PD3)
    GIMSK = _BV(INT0) | _BV(INT1);
    // into and int1 by rising edges
    MCUCR = _BV(ISC11) | _BV(ISC10) | _BV(ISC01) | _BV(ISC00);

    sei();

    while (1) {
    }
}
