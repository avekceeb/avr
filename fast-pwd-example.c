
#define F_CPU 12000000UL

#include <avr/io.h>
#include <util/delay.h>

/* 2 leds changing light in counter-phase */

int main(void) {

    // pulse width of OC1A
    unsigned char widtha = 0;
    // pulse width of OC1B
    unsigned char widthb = 255;

    unsigned char direction = 0;
    
    // PWM output on PB3(OC1A) and PB4(OC1B)  
    DDRB |= (1 << PB3) | (1 << PB4);

    TCCR1A = (1 << COM1A1) | (1 << COM1B1) | (1 << WGM11) | (1 << WGM10);
    TCCR1B = (1 << CS11);
    
    while(1) {
        if ((255 == widtha) || (0 == widtha)) {
           direction ^= 1;
        }
        _delay_ms(5);
        OCR1A = widtha; 
        OCR1B = widthb;
        if (direction) {
            widtha++;
            widthb--;
        } else {
            widtha--;
            widthb++;
        }
   }
}
