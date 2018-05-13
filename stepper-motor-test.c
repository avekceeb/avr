#define F_CPU 12000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>
#include <stdlib.h>

#define SM_port PORTC
#define SM_ddr DDRC

#define IN1 PORTC0
#define IN2 PORTC1
#define IN3 PORTC2
#define IN4 PORTC3

#define SM_delay _delay_ms(5)


void SM_ini(void)
{
	SM_ddr |= (1<<IN4)|(1<<IN3)|(1<<IN2)|(1<<IN1);
	SM_port &= ~((1<<IN4)|(1<<IN3)|(1<<IN2)|(1<<IN1));
}

void SM_set1(void)
{
	SM_port |= 1<<IN1;
	SM_port &= ~(1<<IN2);
	SM_port &= ~(1<<IN3);
	SM_port &= ~(1<<IN4);
	SM_delay;
}

void SM_set2(void)
{
	SM_port |= 1<<IN1;
	SM_port |= 1<<IN4;
	SM_port &= ~(1<<IN2);
	SM_port &= ~(1<<IN3);
	SM_delay;
}

void SM_set3(void)
{
	SM_port |= 1<<IN4;
	SM_port &= ~(1<<IN1);
	SM_port &= ~(1<<IN2);
	SM_port &= ~(1<<IN3);
	SM_delay;
}

void SM_set4(void)
{
	SM_port |= 1<<IN4;
	SM_port |= 1<<IN3;
	SM_port &= ~(1<<IN1);
	SM_port &= ~(1<<IN2);
	SM_delay;
}

void SM_set5(void)
{
	SM_port |= 1<<IN3;
	SM_port &= ~(1<<IN1);
	SM_port &= ~(1<<IN2);
	SM_port &= ~(1<<IN4);
	SM_delay;
}

void SM_set6(void)
{
	SM_port |= 1<<IN3;
	SM_port |= 1<<IN2;
	SM_port &= ~(1<<IN1);
	SM_port &= ~(1<<IN4);
	SM_delay;
}

void SM_set7(void)
{
	SM_port |= 1<<IN2;
	SM_port &= ~(1<<IN1);
	SM_port &= ~(1<<IN3);
	SM_port &= ~(1<<IN4);
	SM_delay;
}

void SM_set8(void)
{
	SM_port |= 1<<IN2;
	SM_port |= 1<<IN1;
	SM_port &= ~(1<<IN3);
	SM_port &= ~(1<<IN4);
	SM_delay;
}

void SM_forvard(void)
{
	SM_set1();
	SM_set2();
	SM_set3();
	SM_set4();
	SM_set5();
	SM_set6();
	SM_set7();
	SM_set8();
}

void SM_back(void)
{
	SM_set8();
	SM_set7();
	SM_set6();
	SM_set5();
	SM_set4();
	SM_set3();
	SM_set2();
	SM_set1();
}

int main(void) {

	int i = 0;
	SM_ini();
    DDRB |= _BV(PB1); 
    
    while(1)
    {
		for (i=0;i<=512;i++)
		{
			SM_forvard();
            PORTB = ~(PORTB);  
		}
		_delay_ms(300);
		for (i=0;i<=512;i++)
		{
			SM_back();
            PORTB = ~(PORTB);  
		}
		_delay_ms(300);
    }
}


