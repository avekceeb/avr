/*
                             attiny2313
                             +---------+
             (RESET/dW) PA2 -| 1     20|- VCC
                  (RXD) PD0 -| 2     19|- PB7 (USCK/SCL/SCK)
                  (TXD) PD1 -| 3     18|- PB6 (MISO/DO)
                (XTAL2) PA1 -| 4     17|- PB5 (MOSI/DI/SDA)
           (CLKI/XTAL1) PA0 -| 5     16|- PB4 (OC1B)
       (CKOUT/XCK/INT0) PD2 -| 6     15|- PB3 (OC1A)
                 (INT1) PD3 -| 7     14|- PB2 (OC0A)
                   (T0) PD4 -| 8     13|- PB1 (AIN1)
              (OC0B/T1) PD5 -| 9     12|- PB0 (AIN0)
                        GND -|10     11|- PD6 (ICPI)
                             +---------+

*/

#define F_CPU 20000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>


//Interrupt 0 Sense Control

#define INT0_BY_LOW_LEVEL 0
#define INT0_BY_ANY_CHANGE (_BV(ISC00))
#define INT0_BY_FALL (_BV(ISC01))
#define INT0_BY_RISE (_BV(ISC01)|_BV(ISC00))


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


void delay(void)
{
	_delay_ms(200);
}

ISR(INT0_vect)
{
    delay();
    mode++;
    mode %= 5;
    TCCR1B = modes[mode];
	PORTD ^= _BV(PD6);
}


int main(void)
{
    // enable interrupts int0 int1 (PD2 PD3)
    GIMSK = _BV(INT0) | _BV(INT1);
    // into and int1 by rising edges
    MCUCR = _BV(ISC11) | _BV(ISC10) | _BV(ISC01) | _BV(ISC00);
    // PWM output on PB3(OC1A) and PB4(OC1B)  
    DDRB |= _BV(PB3) | _BV(PB4);

	// led
	DDRD |= _BV(PD6);

	DDRD &= ~(_BV(PD2) | _BV(PD3));
	// input int0 int1 with pull-ups:
	PORTD |= (_BV(PD2) | _BV(PD3));

    // int0
    MCUCR |= INT0_BY_FALL;

    // enable interrupts int0 int1 (PD2 PD3)
    GIMSK |= (_BV(INT0) | _BV(INT1));


    // WGM 11 12 13 = Fast PWM top=ICR1 Upadate OCR1* at BOTTOM
    // COM1A1+COMB1 = Set OC1A/OC1B on Compare Match, clear OC1A/OC1B at TOP
    TCNT1 = 0x00;
    //ICR1 =  0xff;
    ICR1 = 4;

    // pulse width
    OCR1A = 2;
    OCR1B = 1;
    TCCR1A |= _BV(COM1A1) | _BV(COM1B1) | _BV(WGM11);
    TCCR1B |= _BV(WGM13) | _BV(WGM12) | T1_PRESCALE_1024;

    sei();

    while(1) {
        _delay_ms(5000);
        mode++;
        mode %= 5;
        TCCR1B = modes[mode];
   }
}
