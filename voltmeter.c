/*
                           atmega8
                         +---------+
            (RESET) PC6 -| 1     28|- PC5 (ADC5/SCL)
              (RXD) PD0 -| 2     27|- PC4 (ADC4/SDA)
              (TXD) PD1 -| 3     26|- PC3 (ADC3)
             (INT0) PD2 -| 4     25|- PC2 (ADC2)
             (INT1) PD3 -| 5     24|- PC1 (ADC1)
lcd:d4 <-- (XCK/T0) PD4 -| 6     23|- PC0 (ADC0) <--- Vin
                    VCC -| 7     22|- GND
                    GND -| 8     21|- AREF ----||---|gnd
      (XTAL1/TOSC1) PB6 -| 9     20|- AVCC
      (XTAL2/TOSC2) PB7 -|10     19|- PB5 (SCK)
lcd:d5 <--     (T1) PD5 -|11     18|- PB4 (MISO)     --> lcd:rs
lcd:d6 <--   (AIN0) PD6 -|12     17|- PB3 (MOSI/OC2) --> lcd:en
lcd:d7 <--   (AIN1) PD7 -|13     16|- PB2 (SS/OC1B)
             (ICP1) PB0 -|14     15|- PB1 (OC1A)
                         +---------+

*/

#define ADC0 0
#define ADC1 1
#define ADC2 2
#define ADC3 3
#define ADC4 4
#define ADC5 5

#define F_CPU 16000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <util/delay.h>
#include "lcd.h"
#include "wh1602l-ygk-ct.h"

#define ADC_START_SINGLE_CONVERSION ADCSRA|=_BV(ADSC)

#define ADC_USE_INT_REF 0

#define ADC_REF_VOLTAGE_VCC (5.0f)
#define ADC_REF_VOLTAGE_INT (2.56f)

#if ADC_USE_INT_REF
     // Vref = 2.56
#   define ADC_REF_VOLTAGE ADC_REF_VOLTAGE_INT
#else
    // Vref = Vcc
#   define ADC_REF_VOLTAGE ADC_REF_VOLTAGE_VCC
#endif


ISR (ADC_vect)
{
    // ADCW contains converted value
    float value = ADCW * (ADC_REF_VOLTAGE/1024.0);
    char buffer [16];
    sprintf(buffer, " value = %.2f V ", value);
    wh1602_print_upper(buffer);
    _delay_ms(10);
    ADC_START_SINGLE_CONVERSION;
}


void adc_init(void)
{
#if ADC_USE_INT_REF
    ADMUX |= ( _BV(REFS1) | _BV(REFS0) );
#else
    ADMUX |= _BV(REFS0);
#endif
    ADMUX |= ADC0;
    // EN - adc enable
    // IE - interrupt enable
    // PS0..2 - prescaler = 128
    ADCSRA |= _BV(ADEN) | _BV(ADIE) | _BV(ADPS0) | _BV(ADPS1) | _BV(ADPS2);
}


int main(void)
{
    // using 'default' lcd connections from lcd.h
    lcd_init();
    adc_init();
    sei();
    ADC_START_SINGLE_CONVERSION;
    while (1) {
        _delay_ms(10);
    }
}
