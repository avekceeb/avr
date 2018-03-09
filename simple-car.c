
#define F_CPU 12000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/wdt.h>

/*
 * Simple autonomous car (variant 01.az)
 * 2-wheel drive
 * One axis is connected to optical shaft encoder
 * Car chaotically dances in loop:
 *    forward -> right -> backward -> left
 */

/*
 * PB0
 * PB1
 * PB2 (OCOA)   ---> right pwm (enable)
 * PB3
 * PB4
 * PB5
 * PB6          ---> right bridge fwd
 * PB7          ---> right bridge bkw
 * PD0          ---> left  bridge fwd
 * PD1          ---> left  bridge bkw
 * PD2 (INT0)   <--- shaft encoder
 * PD3
 * PD4
 * PD5 (OC0B)   ---> left pwm (enable)
 * PD6
 */

// common helpers:
#define set_bit(port,bit)   (port |= (bit))
#define clear_bit(port,bit) (port &= ~(bit))

// connections:
#define right_port OCR0A
#define right_fwd _BV(PB6)
#define right_bkw _BV(PB7)
#define left_port  OCR0B
#define left_fwd  _BV(PD0)
#define left_bkw  _BV(PD1)

// H-bridge controls:
// left:
#define stop_left \
    clear_bit(PORTD, left_fwd) ;\
    clear_bit(PORTD, left_bkw)
#define fwd_left \
    set_bit(PORTD, left_fwd) ;\
    clear_bit(PORTD, left_bkw)
#define bkw_left \
    set_bit(PORTD, left_bkw) ;\
    clear_bit(PORTD, left_fwd)
// right      
#define stop_right \
    clear_bit(PORTB, right_fwd) ;\
    clear_bit(PORTB, right_bkw)
#define fwd_right \
    set_bit(PORTB, right_fwd) ;\
    clear_bit(PORTB, right_bkw)
#define bkw_right \
    set_bit(PORTB, right_bkw) ;\
    clear_bit(PORTB, right_fwd)

// H-bridge + PWM controls:
#define move_fwd(pwm)   fwd_left;\
                        fwd_right;\
                        right_port=pwm;\
                        left_port=pwm;
#define move_bkw(pwm)   bkw_left;\
                        bkw_right;\
                        right_port=pwm;\
                        left_port=pwm;
#define turn_left(pwm)  fwd_left;\
                        bkw_right;\
                        right_port=pwm;\
                        left_port=pwm;
#define turn_right(pwm) bkw_left;\
                        fwd_right;\
                        right_port=pwm;\
                        left_port=pwm;
#define stop    right_port=0;\
                left_port=0;

// constants
#define long_trip  800
#define short_trip 400 
#define full_speed 0xff
#define slow_speed 0xef

volatile unsigned int shaft_counter = 0;
volatile unsigned int counting = long_trip;
volatile unsigned char direction = 0x00;

void change_direction() {
    // should change direction no less often than very 8 sec
    wdt_reset();
    stop;
    if (++direction >= 4) {
        direction = 0;
    }
    _delay_ms(500);
    switch (direction) {
        case 0:
            counting = long_trip;
            move_fwd(full_speed);
            break;
        case 1:
            counting = short_trip;
            turn_right(full_speed);
            break;
        case 2:
            counting = long_trip;
            move_bkw(full_speed);
            break;
        case 3:
            counting = short_trip;
            turn_left(full_speed);
            break;
        default:
            stop;
    }
}

ISR(INT0_vect) {
    if (++shaft_counter >= counting) {
        shaft_counter = 0;
        change_direction();
    }
}

int main() {
    DDRB = right_bkw | right_fwd | _BV(PB2);
    
    DDRD = left_bkw | left_fwd | _BV(PD5);
    set_bit(PORTD, (_BV(PD2)|_BV(PD3)));

    // enable interrupts int0 int1 (PD2 PD3)
    GIMSK = _BV(INT0) | _BV(INT1);
    // into and int1 by rising edges
    MCUCR = _BV(ISC11) | _BV(ISC10) | _BV(ISC01) | _BV(ISC00);

    TCCR0A = _BV(COM0A1) | _BV(COM0B1) | _BV(WGM00);

    TCCR0B = (1 << CS01);

    wdt_enable(WDTO_8S);

    _delay_ms(2000);

    move_fwd(full_speed);

    sei();

    for (;;) {
    }

    return 0;

}
