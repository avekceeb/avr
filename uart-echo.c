#define F_CPU 12000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <stdint.h>
#include <stdio.h>

#define BAUD 9600
//#define BAUD 2400
// asyncronous normal mode:
#define MYUBRR (F_CPU/16/BAUD-1)

#define use_seven_segment 0
/*
 * Sample program to receive byte from UART
 * then echo it back
 * and display on LED (only 6 segments - wrong port B choosen)
 *
 * PB0 ---> a
 * PB1 ---> b
 * PB2 ---> c
 * PB3 ---> d
 * PB4 ---> e
 * PB5 ---> f
 * PB6 ---> g
 * PB7 ---> h 
 * PD0 <--- Rx
 * PD1 ---> Tx
 * PD2
 * PD3
 * PD4
 * PD5
 * PD6
 */

/*
       a   
    +-----+
   f|     |b
    |  g  |
    +-----+
   e|     |c
    |     |
    +-----+   * h
       d
*/

#if use_seven_segment
static const unsigned char segments[16] PROGMEM = {
	/* hgfedcba   */
	/* 00111111 0 */ 0x3f,
	/* 00000110 1 */ 0x06,
	/* 01011011 2 */ 0x5b,
	/* 01001111 3 */ 0x4f,
	/* 01100110 4 */ 0x66,
	/* 01101101 5 */ 0x6d,
	/* 01111101 6 */ 0x7d,
	/* 00000111 7 */ 0x07,
	/* 01111111 8 */ 0x7f,
	/* 01101111 9 */ 0x6f,
	/* 01011111 a */ 0x5f, 
	/* 01111100 b */ 0x7c, 
	/* 00111001 c */ 0x39, 
	/* 01011110 d */ 0x5e,
	/* 01111001 e */ 0x79,
	/* 01110001 f */ 0x71
};
#endif

uint8_t rx_data = 0;
char * message = "hello from mc\r\n";
uint8_t message_sz = 15;

void transmit(uint8_t data) {
    while (!(UCSRA & _BV(UDRE)));
    UDR = data;
}

#ifdef __AVR_ATmega8__
ISR(USART_RXC_vect) {
#else
ISR(USART_RX_vect) {
#endif
    rx_data = UDR;
#if use_seven_segment
    PORTB = pgm_read_byte(
                &(segments[rx_data & 0x0f]));
#endif
    transmit(rx_data);
}


int main(void) {
    // set PB as output
    DDRB = 0xff;
    PORTB = 0x00;

	/* Set baud rate */
	UBRRH = (unsigned char)(MYUBRR>>8);
	UBRRL = (unsigned char)MYUBRR;
	/* Enable receiver and transmitter and Rx Int*/
	UCSRB = _BV(RXEN) | _BV(TXEN) | _BV(RXCIE); // | TXIE
#ifdef __AVR_ATmega8__
	// 8 bit ; 1 stop ; no parity ; asynchronous
    UCSRC = _BV(URSEL) | _BV(UCSZ1) | _BV(UCSZ0);
#else
	UCSRC = (1 << USBS) | (3 << UCSZ0);	// asynchron 8n1
#endif
	/* Set frame format: 8data, 2stop bit */
	//UCSRC = (1<<URSEL)|(1<<USBS)|(3<<UCSZ0);
    
    sei();
    uint8_t i;
    while(1) {
        PORTB = ~(PORTB);
        _delay_ms(2000);
        for (i=0; i<message_sz; i++) {
            transmit(message[i]);
        }
    }
}

