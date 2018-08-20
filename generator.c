
#define F_CPU 20000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

// Simple pulse generator

#define T1_PRESCALE_1024 (_BV(CS12)|_BV(CS10))
#define T1_PRESCALE_256  (_BV(CS12))
#define T1_PRESCALE_64   (_BV(CS11)|_BV(CS10))
#define T1_PRESCALE_8    (_BV(CS11))
#define T1_PRESCALE_1    (_BV(CS10))


uint8_t mode = 0;

uint8_t modes[5] = {
    _BV(WGM13)|_BV(WGM12)|T1_PRESCALE_1024,
    _BV(WGM13)|_BV(WGM12)|T1_PRESCALE_256,
    _BV(WGM13)|_BV(WGM12)|T1_PRESCALE_64,
    _BV(WGM13)|_BV(WGM12)|T1_PRESCALE_8,
    _BV(WGM13)|_BV(WGM12)|T1_PRESCALE_1,
};

ISR(INT0_vect)
{
    _delay_ms(100);
    mode++;
    mode %= 5;
    TCCR1B = modes[mode];
}


int main(void)
{
    // enable interrupts int0 int1 (PD2 PD3)
    GIMSK = _BV(INT0) | _BV(INT1);
    // into and int1 by rising edges
    MCUCR = _BV(ISC11) | _BV(ISC10) | _BV(ISC01) | _BV(ISC00);
    // PWM output on PB3(OC1A) and PB4(OC1B)  
    DDRB |= _BV(PB3) | _BV(PB4);
    // WGM 11 12 13 = Fast PWM top=ICR1 Upadate OCR1* at BOTTOM
    // CS11 = prescale 8
    // COM1A1+COMB1 = Set OC1A/OC1B on Compare Match, clear OC1A/OC1B at TOP
    TCNT1 = 0x00;
    //ICR1 =  0xff;
    ICR1 = 4;
    // pulse width
    OCR1A = 2;
    OCR1B = 1;
    TCCR1A |= _BV(COM1A1) | _BV(COM1B1) | _BV(WGM11);
    TCCR1B |= _BV(WGM13) | _BV(WGM12) | T1_PRESCALE_1024;
    
    while(1) {
        _delay_ms(5000);
        mode++;
        mode %= 5;
        TCCR1B = modes[mode];
   }
}
