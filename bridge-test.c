/*
                             attiny2313
                             +---------+
             (RESET/dW) PA2 -| 1     20|- VCC
                  (RXD) PD0 -| 2     19|- PB7 (USCK/SCL/SCK)
                  (TXD) PD1 -| 3     18|- PB6 (MISO/DO)
                (XTAL2) PA1 -| 4     17|- PB5 (MOSI/DI/SDA)
           (CLKI/XTAL1) PA0 -| 5     16|- PB4 (OC1B)  --------> PWM_L
       (CKOUT/XCK/INT0) PD2 -| 6     15|- PB3 (OC1A)  --------> PWM_R
                 (INT1) PD3 -| 7     14|- PB2 (OC0A)
                   (T0) PD4 -| 8     13|- PB1 (AIN1)  --------> BKW_R
              (OC0B/T1) PD5 -| 9     12|- PB0 (AIN0)  --------> FWD_R
                        GND -|10     11|- PD6 (ICPI)
                             +---------+

*/

#define F_CPU 16000000UL

#include <stdint.h>
#include <avr/io.h>
#include <util/delay.h>

#define PWM_L OCR1B
#define PWM_R OCR1A
#define BKW_R PB1
#define FWD_R PB0

int main(void) {

    // PWM output on PB3(OC1A) and PB4(OC1B)  
    DDRB |= _BV(PB3) | _BV(PB4) | _BV(BKW_R) | _BV(FWD_R);
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
        // 1) full speed fwd
        PORTB &= ~(_BV(BKW_R));
        PORTB |= _BV(FWD_R);
        PWM_R = 0;
        PWM_L = 0xff; // ...
        _delay_ms(1500);
        // 2) stop
        PORTB &= ~(_BV(BKW_R));
        PORTB &= ~(_BV(FWD_R));
        _delay_ms(600);
        // 3) slow speed bkw
        PORTB &= ~(_BV(FWD_R));
        PORTB |= _BV(BKW_R);
        PWM_R = 0x30;
        PWM_L = 0xd0; // ...
    }
}
