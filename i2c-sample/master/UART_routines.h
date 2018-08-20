//**************************************************************
//******** FUNCTIONS FOR SERIAL COMMUNICATION USING UART *******
//**************************************************************
//Controller: ATmega32 (Clock: 8 Mhz-internal)
//Compiler	: AVR-GCC (winAVR with AVRStudio)
//Version 	: 2.3
//Author	: CC Dharmani, Chennai (India)
//			  www.dharmanitech.com
//Date		: 08 May 2010
//**************************************************************

//**************************************************
// ***** HEADER FILE : UART_routines.h ******
//**************************************************

#ifndef _UART_ROUTINES_H_
#define _UART_ROUTINES_H_

#define CHAR 0
#define INT  1
#define LONG 2

#define	SYSCLK 4000000UL
#define BAUDRATE 19200L
#define baud SYSCLK/(16*BAUDRATE)-1
#define HI(x) ((x)>>8)
#define LO(x) ((x)&0xFF)

#define TX_NEWLINE {transmitByte(0x0d); transmitByte(0x0a);}

void uart0_init(void);
unsigned char receiveByte(void);
void transmitByte(unsigned char);
void transmitString_F(char*);
void transmitString(unsigned char*);
void transmitHex( unsigned char dataType, unsigned long data );

#define sendStr(x) transmitString_F(PSTR(x));
#define sendStr0(x) {transmitString_F(PSTR(x));TX_NEWLINE;}
#define sendStrF(x) {transmitString(x)}
#define sendStrF0(x) {transmitString(x);TX_NEWLINE;}

#endif
