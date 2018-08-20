#define F_CPU 4000000UL

#include <string.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <stdio.h>
#include <avr/interrupt.h>

#include "i2c.h"
#include "UART_routines.h"

#define LOCAL_ADDR 0x07
#define TARGET_ADDR 0xA0 

unsigned char buf[40];

// slave operations 
void i2cSlaveReceiveService(u08 receiveDataLength, u08* receiveData) 
{ 
   sprintf(buf,"slave received: %s (length %d)",receiveData,receiveDataLength);
   transmitString(buf);TX_NEWLINE;
} 

int main(void)
{
	uart0_init();
	//_delay_ms(10);
	sendStr0("*************TWI SLAVE RUNNING*************");

	// initialize i2c function library 
  	i2cInit(); 
  	// set local device address and allow response to general call 
  	i2cSetLocalDeviceAddr(LOCAL_ADDR, TRUE); 
	
	i2cSetSlaveReceiveHandler( i2cSlaveReceiveService ); 

	while(1)
	{
		_delay_ms(100);
	}
}
