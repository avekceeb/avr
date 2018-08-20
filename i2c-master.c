
// http://playingwithatmega8.blogspot.com/2016/05/twi-or-i2c-communication.html

/*
                       atmega8
                     +---------+
        (RESET) PC6 -| 1     28|- PC5 (ADC5/SCL)
          (RXD) PD0 -| 2     27|- PC4 (ADC4/SDA)
          (TXD) PD1 -| 3     26|- PC3 (ADC3)
         (INT0) PD2 -| 4     25|- PC2 (ADC2)
         (INT1) PD3 -| 5     24|- PC1 (ADC1)
       (XCK/T0) PD4 -| 6     23|- PC0 (ADC0)
                VCC -| 7     22|- GND
                GND -| 8     21|- AREF
  (XTAL1/TOSC1) PB6 -| 9     20|- AVCC
  (XTAL2/TOSC2) PB7 -|10     19|- PB5 (SCK)
           (T1) PD5 -|11     18|- PB4 (MISO)
         (AIN0) PD6 -|12     17|- PB3 (MOSI/OC2)
         (AIN1) PD7 -|13     16|- PB2 (SS/OC1B)
         (ICP1) PB0 -|14     15|- PB1 (OC1A)
                     +---------+

*/


#include <avr/io.h>
#define F_CPU 16000000UL
#include <util/delay.h>

#include <avr/interrupt.h>
#include <util/twi.h>

#define SLAVE_ADDRESS (0x01)

uint8_t value; // contains the received value

uint8_t ongoing_transmission = 0;

// interrupt routine for the timer0 overflow interrupt
ISR(TWI_vect)
{
  // react on TWI status and handle different cases
  uint8_t status = TWSR & 0xFC; // mask-out the prescaler bits
  switch(status)
  {
    case TW_START:  // start transmitted
         ongoing_transmission = 1;
         // write SLA+R, SLA=0x01
         TWDR = (SLAVE_ADDRESS << 1) | 0x01;
         TWCR &= ~((1<<TWSTA)); // clear TWSTA
    break;
  
    case TW_MR_SLA_ACK: // SLA+R transmitted, ACK received 
         TWCR &= ~((1<<TWSTA) | (1<<TWSTO)); 
    break;
  
    case TW_MR_DATA_ACK: // data received, ACK returned
         ongoing_transmission = 0;
         value = TWDR;
         TWCR |= (1<<TWSTO);  // write stop bit
         TWCR &= ~(1<<TWSTA); // clear start bit
    break;
  }
  TWCR |=   (1<<TWINT);  // hand over to TWI hardware
}

int main(void) 
{
  // TWI setup
  sei(); // enable global interrupt
  // TWI-ENable , TWI Interrupt Enable
  TWCR |= (1<<TWEA) | (1<<TWEN) | (1<<TWIE); 

  // LED setup
  DDRB  = 0x3f; // PORTB[0:5] as output
  DDRC  = 0x03; // PORTC[0:1] as output
 
  for (;;) // infinite main loop
  {
    // initiate new transmission if 
    //    no transmission is in progress
    if (!ongoing_transmission) TWCR |= (1<<TWSTA); 
 
    PORTB = value;    // display number
    PORTC = value>>6; //   as LED pattern 
  }
}
