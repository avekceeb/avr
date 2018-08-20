#include <avr/io.h>
#include <avr/interrupt.h>

#define F_CPU 16000000UL
#include <util/delay.h>
#include <util/twi.h>

#define SLAVE_ADDRESS (0x01)

uint8_t adc_value; // the value to send

// interrupt routine for TWI message handling
ISR(TWI_vect)
{
  // react on TWI status and handle different cases
  uint8_t status = TWSR & 0xFC; // mask-out the prescaler bits
  switch(status)
  {
    case TW_ST_SLA_ACK:   // own SLA+R received, acknoledge sent
         TWDR = adc_value;
         TWCR &= ~((1<<TWSTO) | (1<<TWEA));
    break;
    
    case TW_ST_LAST_DATA: // last byte transmitted ACK received     
         TWCR |= (1<<TWEA); // set TWEA to enter slave mode
    break;
    }
    TWCR |= (1<<TWINT);  // set TWINT -> activate TWI hardware
}

int main(void)
{
  // TWI setup
  sei(); // enable global interrupt
 
  // set slave address to 0x01, ignore general call
  TWAR = (SLAVE_ADDRESS << 1) | 0x00;
  // TWI-ENable , TWI Interrupt Enable
  TWCR |= (1<<TWEA) | (1<<TWEN) | (1<<TWIE); 
    
  // ADC setup
  ADCSRA |= (1<<ADEN);
  ADMUX  |= ( (1<<REFS1) | (1<<REFS0) ); // select internal reference
  ADMUX  |= 3;   // select channel 3 (pin ADC3)
 
  // infinite loop
  for (;;)
  {
    ADCSRA |= (1<<ADSC);        // start single measurement
    while(ADCSRA & (1<<ADSC));  // wait until measurement done
    
    // read result bytes (low and high) and reduce to 8-bits
    adc_value = ADCL;
    adc_value >>= 2;          // drop 2 least significant bits
    adc_value |= (ADCH << 6); // add two most significant bits
  }
}
