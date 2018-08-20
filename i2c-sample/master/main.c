#define F_CPU 4000000UL

#include <string.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <stdio.h>
#include <avr/interrupt.h>

#include "i2c.h"
#include "UART_routines.h"

#define LOCAL_ADDR 0xA0 
#define TARGET_ADDR 0x07 

unsigned char buf[100];

int main(void)
{
	uart0_init();
	_delay_ms(1000);
	sendStr0("*************TWI MASTER RUNNING*************");

	// initialize i2c function library 
  	i2cInit(); 
  	// set local device address and allow response to general call 
  	i2cSetLocalDeviceAddr(LOCAL_ADDR, TRUE); 

	unsigned char mBuf[100];
	sprintf(mBuf,"%s","hello!");mBuf[strlen(mBuf)]=0x00;
    i2cMasterSend(TARGET_ADDR,strlen(mBuf)+1/*(0x00)*/,&mBuf[0]);

    sprintf(buf,"master sended: %s (length %d)",mBuf,strlen(mBuf)+1/*(0x00)*/);
    transmitString(buf);TX_NEWLINE;

	while(1)
	{
		_delay_ms(100);
	}
}
