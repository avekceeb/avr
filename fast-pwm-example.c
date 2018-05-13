
#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>

/* 2 leds changing light in counter-phase */

int main(void) {

    // pulse width of OC1A
    unsigned char widtha = 0;
    // pulse width of OC1B
    unsigned char widthb = 0xff;

    unsigned char a_up_b_down = 1;
    
    // PWM output on PB3(OC1A) and PB4(OC1B)  
    DDRB |= (1 << PB3) | (1 << PB4);

    // WGM 11 12 13 = Fast PWM top=ICR1 Upadate OCR1* at BOTTOM
    // CS11 = prescale 8
    // COM1A1+COMB1 = Set OC1A/OC1B on Compare Match, clear OC1A/OC1B at TOP
    TCNT1 = 0x00;
    ICR1 =  0xff;
    OCR1A = 0x00;
    OCR1B = 0x00;
    TCCR1A |= _BV(COM1A1) | _BV(COM1B1) | _BV(WGM11);
    TCCR1B |= _BV(WGM13) | _BV(WGM12) | _BV(CS11);
    
    while(1) {
        _delay_ms(10);
        OCR1A = widtha;
        OCR1B = widthb;
        if (a_up_b_down) {
            widtha++;
            widthb--;
        } else {
            widtha--;
            widthb++;
        }
        if ((0 == widtha) || (0xff == widtha)) {
           a_up_b_down ^= 1;
        }
   }
}
