
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


#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include <stdio.h>
#include "ir-nec.h"
#include "yamaha-cdx4.h"
#include "sport-tablo.h"

#define tsop PD3

//#define BAUD 9600
#define BAUD 2400
// asyncronous normal mode:
#define MYUBRR (F_CPU/16/BAUD-1)

#define pos_cmd 2

#define pos_score_left_h 4
#define pos_score_left_l 5

#define pos_score_right_h 7
#define pos_score_right_l 8

#define pos_period 8

#define MSG_SZ 11
#define MSG_PERIOD_SZ 9

#define skip_repeat 1

struct IR_Packet received_packet;

uint8_t rx_data = 0;

uint8_t score_left, score_right, period;

uint8_t message[MSG_SZ] = {
    START_BYTE_1,
    START_BYTE_2,
    COMMAND_SCORE,
    0x20,
    1,
    2,
    0x20,
    3,
    4,
    0,
    0
};


void update_1_digit_bcd(uint8_t value, uint8_t* bcd) {
    if (value > 9) {
        return;
    }
    *bcd = value;
}

void update_2_digit_bcd(uint8_t value, uint8_t* hi, uint8_t* lo) {
    if (value > 99) {
        return;
    }
    *lo = value % 10;
    *hi = (value - *lo) / 10;
}

uint8_t wrapped_increment_1_digit(uint8_t value) {
    value++;
    if (value <= 9) {
        return value;
    } else {
        return 0;
    }
} 

uint8_t wrapped_decrement_1_digit(uint8_t value) {
    if ((value == 0) || (value > 9)) {
        return 9;
    } else {
        return value-1;
    }
} 

uint8_t wrapped_increment_2_digit(uint8_t value) {
    value++;
    if (value <= 99) {
        return value;
    } else {
        return 0;
    }
} 

uint8_t wrapped_decrement_2_digit(uint8_t value) {
    if ((value == 0) || (value > 99)) {
        return 99;
    } else {
        return value-1;
    }
} 


void transmit(uint8_t data) {
    while (!(UCSRA & _BV(UDRE)));
    UDR = data;
}


void send_score() { 
    uint8_t i;
    update_2_digit_bcd(
        score_left,
        &(message[pos_score_left_h]),
        &(message[pos_score_left_l]));
    update_2_digit_bcd(
        score_right,
        &(message[pos_score_right_h]),
        &(message[pos_score_right_l]));
    message[pos_cmd] = COMMAND_SCORE;
    for (i=0; i<MSG_SZ; i++) {
        transmit(message[i]);
    }
}

void send_period() { 
    update_1_digit_bcd(
        period, &(message[pos_period]));
    message[pos_cmd] = COMMAND_PERIOD;
    uint8_t i;
    for (i=0; i<MSG_PERIOD_SZ; i++) {
        transmit(message[i]);
    }
}


int main(void) {
    // set PB as output
    DDRB = 0xff;
    PORTB = 0xff;

    // TxD
    DDRD |= _BV(PD1);

    /* Set baud rate */
    UBRRH = (unsigned char)(MYUBRR>>8);
    UBRRL = (unsigned char)MYUBRR;
    /* Enable only transmitter*/
    UCSRB = _BV(TXEN);
#ifdef __AVR_ATmega8__
    // 8 bit ; 1 stop ; no parity ; asynchronous
    UCSRC = _BV(URSEL) | _BV(UCSZ1) | _BV(UCSZ0);
#else
    UCSRC = (1 << USBS) | (3 << UCSZ0);// asynchron 8n1
#endif

    score_left = 0;
    score_right = 0;
    period = 1;

    _delay_ms(1500);

    send_score();
    PORTB = 0;
    _delay_ms(1500);

    send_period();
    PORTB = 0xff;
    _delay_ms(1500);

    init_receiver();
    PORTB = 0;

    while (1) {
        cli();
        uint8_t check_result = check_new_packet(&received_packet);
        sei();
        if (check_result) {
#if skip_repeat
            if (received_packet.repeat) {
                continue;
            }
#endif
            uint8_t i;
            for (i=2; i<MSG_SZ; i++) {
                message[i] = 0;
            }
            PORTB = ~(PORTB);
            switch (received_packet.command) {
                // LEFT++
                case button_repeat:
                    score_left = wrapped_increment_2_digit(score_left);
                    send_score();
                    break;
                // RIGHT++
                case button_random:
                    score_right = wrapped_increment_2_digit(score_right);
                    send_score();
                    break;
                // PERIOD++
                case button_pause:
                    period = wrapped_increment_1_digit(period);
                    send_period();
                    break;
                // LEFT--
                case button_fast_backward:
                    score_left = wrapped_decrement_2_digit(score_left);
                    send_score();
                    break;
                // RIGHT--
                case button_fast_forward:
                    score_right = wrapped_decrement_2_digit(score_right);
                    send_score();
                    break;
                // PERIOD--
                case button_stop:
                    period = wrapped_decrement_1_digit(period);
                    send_period();
                    break;

            }
        }
    }

}

