
#define F_CPU 12000000UL

#include <avr/io.h>
#include <avr/interrupt.h>

char global_char = 5;

unsigned char global_uchar = 6;

int main(void) {

    float local_float = 3.14; 

    char local_char = 7;

    DDRB |= _BV(PB0);

    sei();

    while (1) {
        local_char += 3; 
        PORTB = global_char++;
        PORTD = local_char--;
        PORTC = global_uchar++;
        local_float *= 1.05;
    }
}

