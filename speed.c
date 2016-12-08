/*
 * GccApplication2.c
 *
 * Created: 2016-11-24 19:42:33
 *  Author: tmk16lh
 */ 
#define LED_LEFT	(1 << PB0)
#define LED_RIGHT	(1 << PD7)
#define PIN_INT		(1 << PB1)

#include <avr/io.h>
#include <avr/interrupt.h>

static volatile int intCnt	= 0;
static volatile char speed;
static volatile int tmp;

void io_init(void)
{
	DDRB	= LED_LEFT;
	DDRD	= LED_RIGHT | (1 << PD3);
	PORTB	= PIN_INT;
}

void interrupt_init(void)
{
	PCICR	= (1 << PCIE0);
	PCMSK0	= (1 << PCINT1);
}

void timer_init(void)
{
	TCCR1B	= (1 << WGM12);
	OCR1A	= 15625; //1 seconds
	TIMSK1	= (1 << OCIE1A);
	TCCR1B	|= (1 << CS11) | (1 << CS10);
}

void pwm_init(void)
{
	TCCR2A |= (1 << COM0B1);
	TCCR2A |= (1 << WGM01)|(1 << WGM00);
	TCCR2B |= (1 << CS20);

	OCR2B = 100; //DUTY

}

int main(void)
{
	io_init();
	interrupt_init();
	timer_init();
	pwm_init();
	sei();
	
    while(1)
    {
    	if(speed > 70 && speed < 90)
    	{
    		PORTB |= LED_LEFT;
    	}
    	else
    	{
    		PORTB &= ~LED_LEFT;
    	}
    }
}

ISR(PCINT0_vect)
{
	
	//if((PINB & 4) > 0){
		//PORTD ^= LED_RIGHT;
	intCnt = intCnt + 1;
	//}
}

ISR(TIMER1_COMPA_vect)
{

	if(intCnt > 1000)
	{
		PORTD |= LED_RIGHT;
	}
	else
	{
		PORTD &= ~LED_RIGHT;
	}

	tmp = intCnt;
	intCnt = 0;
	speed = (unsigned char) tmp*60/(48.0);
}