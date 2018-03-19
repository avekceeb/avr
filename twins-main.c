/*
 * Simple toy car
 * 2 brush motors on H-bridge
 * variant 01.vedi
                               atmega8
                             +---------+
                (RESET) PC6 -| 1     28|- PC5 (ADC5/SCL)
  command ------> (RXD) PD0 -| 2     27|- PC4 (ADC4/SDA)
  [reply] <----x  (TXD) PD1 -| 3     26|- PC3 (ADC3) ----> fwd R
  [obstacle L]x->(INT0) PD2 -| 4     25|- PC2 (ADC2) ----> bkw R
  [obstacle R]x->(INT1) PD3 -| 5     24|- PC1 (ADC1) ----> fwd L
  enc L -----> (XCK/T0) PD4 -| 6     23|- PC0 (ADC0) ----> bkw L
                        VCC -| 7     22|- GND
                        GND -| 8     21|- AREF
          (XTAL1/TOSC1) PB6 -| 9     20|- AVCC
          (XTAL2/TOSC2) PB7 -|10     19|- PB5 (SCK)
  enc R ------->   (T1) PD5 -|11     18|- PB4 (MISO)
  led L <------- (AIN0) PD6 -|12     17|- PB3 (MOSI/OC2) ---> pwm for R & L
  led R <------- (AIN1) PD7 -|13     16|- PB2 (SS/OC1B)
                 (ICP1) PB0 -|14     15|- PB1 (OC1A)
                             +---------+

*/

#define F_CPU 12000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>

// ---- helpers ------
#define set_bit(port,bit)   (port |= _BV(bit))
#define clear_bit(port,bit) (port &= ~(_BV(bit)))

// ---- connections -----
// inputs:
#define rxd  PD0
#define obsL PD2
#define obsR PD3
#define encL PD4
#define encR PD5
// outputs:
#define txd  PD1
#define fwdR PD3
#define bkwR PD2
#define fwdL PC1
#define bkwL PC0
#define pwm  PB3
#define ledL PD6
#define ledR PD7

#define port_bridge PORTC
#define register_pwm OCR2

// ----- command executions -----
#define set_direction(cmd) (port_bridge = cmd)
#define set_speed(speed) (register_pwm = speed);

// ---- h-bridge port --------
#define move_fwd_cmd   0b00001010
#define move_bkw_cmd   0b00000101
#define turn_left_cmd  0b00000110
#define turn_right_cmd 0b00001001
#define break_all_cmd  0b00000000

// ----- command codes ------
// ir remote control for Yamaha CDX4
#define cmd_invalid 0x00
#define cmd_fwd 0x55
#define cmd_bkw 0x56
#define cmd_left 0x04
#define cmd_right 0x07
#define cmd_stop 0x02
#define cmd_speedup 0x1d
#define cmd_slowdown 0x1c

// ---- usart settings ------
#define baudrate 9600
// asyncronous normal mode:
#define ubrr (F_CPU/16/baudrate-1)

// ---- transmission settings
// 2 rounds of wheel
#define left_encoder (0xff-12) 
#define right_encoder (0xff-12) 

// shaft encoder L Timer0 - 8bit
ISR (TIMER0_OVF_vect) {
    // TODO: wdt reset 
    PORTD ^= _BV(ledL);
    TCNT0 = left_encoder;
}
// shaft encoder R Timer1 - 16bit
ISR (TIMER1_OVF_vect) {
    PORTD ^= _BV(ledR);
    TCNT1H = 0xff;
    TCNT1L = right_encoder;
}

uint8_t command;
uint8_t newcommand;

ISR(USART_RXC_vect) {
    command = UDR;
    newcommand = 0x01;
}

int main(void) {

    // 1) configure IO ports
    // pull-ups for inputs
    PORTD = _BV(rxd) | _BV(obsL) | _BV(obsR) | _BV(encL) | _BV(encR);
    // set outputs:
    DDRD = _BV(ledL) | _BV(ledR);
    DDRB = _BV(pwm);
    DDRC = _BV(fwdR) | _BV(bkwR) | _BV(fwdL) | _BV(bkwL);

    // configure USART
    UBRRH = (uint8_t)(ubrr>>8);
    UBRRL = (uint8_t)ubrr;
    // Enable receiverand Rx Int
    UCSRB = _BV(RXEN) | _BV(RXCIE); // | _BV(TXEN) | TXIE
    // 8 bit ; 1 stop ; no parity ; asynchronous
    UCSRC = _BV(URSEL) | _BV(UCSZ1) | _BV(UCSZ0);

    // 2) configure timers
    // enable timer interrupt
    TIMSK = _BV(TOIE0) | _BV(TOIE1);
    // Counter0:
    // External clock source on T0 pin. Clock on rising edge
    TCCR0 = _BV(CS02) | _BV(CS01) | _BV(CS00);
    // Counter1: normal mode
    // External clock source on T1 pin. Clock on rising edge
    TCCR1B = _BV(CS12) | _BV(CS11) | _BV(CS10);

    TCNT0 = left_encoder;
    TCNT1H = 0xff;
    TCNT1L = right_encoder;

    // 4) set phase correct pwm for Counter2 (non-invert)
    // Clear OC2 on Compare Match when up-counting.
    // Set OC2 on Compare Match when downcounting
    // f = Fclk/(N*512) = 23kHz/N N - prescaler
    // lets start with N=1
    TCCR2 = _BV(WGM20)/*pwm*/ | _BV(COM21) | _BV(CS20);
    // TODO: CS22 CS21 CS20 as 000 => stop counter

    // initial state of h-bridge: stop at full speed
    port_bridge = 0;
    uint8_t speed = 0xdd;
    uint8_t direction = break_all_cmd;
    command = cmd_invalid;
    newcommand = 0x01;

    sei();
    
    while (1) {
        if (newcommand) {
            switch(command) {
                case cmd_fwd:
                case 'f':
                    direction = move_fwd_cmd; break;
                case 'b':
                case cmd_bkw:
                    direction = move_bkw_cmd; break;
                // TODO: limited movement
                case 'l':
                case cmd_left:
                    direction = turn_left_cmd; break;
                case 'r':
                case cmd_right:
                    direction = turn_right_cmd; break;
                case 's':
                case cmd_stop:
                    direction = break_all_cmd; break;
                case '+':
                case cmd_speedup:
                    if (speed < 0xff) {
                        speed++;
                    }
                    break;
                case '-':
                case cmd_slowdown:
                    if (speed > 0x00) {
                        speed--;
                    }
                    break;
            }
            set_direction(direction);
            set_speed(speed);
            newcommand = 0;
        }
    }
    return 0;
}
