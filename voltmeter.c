
#define F_CPU 16000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <util/delay.h>
#include "lcd.h"
#include "wh1602l-ygk-ct.h"

#define start_onetime_adc ADCSRA|=_BV(ADSC)


ISR (ADC_vect)
{
    // ADCW contains converted value
    float value = ADCW*0.00489;
    char buffer [16];
    sprintf(buffer, " value = %.2f V ", value);
    lcd_command(lcd_goto_upper_line);
    for (uint8_t i=0;i<16;i++) {
        lcd_data(buffer[i]);
    }
    _delay_ms(10);
    start_onetime_adc;
}


void adc_init(void)
{
    // AVcc - reference vlotage
    ADMUX |=(1<<REFS0);
    ADCSRA |=(1<<ADEN)|(1<<ADIE)|(1<<ADPS0)|(1<<ADPS1)|(1<<ADPS2);
}


int main(void)
{
    lcd_init();
    adc_init();
    sei();
    start_onetime_adc;

    while (1) {
        _delay_ms(10);
    }
}
