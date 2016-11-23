#include <avr/io.h>
#include<avr/interrupt.h>

int main(void)
{
	sei();
	PRR &= ~(1 << 6);
	TIMSK2 |= (1<<2)|(1<<1)|(1<<0);
	TCCR2A &= ~((1<<5)|(1<<4)|(1<<1)|(1<<0));
	TCCR2B &= ~((1<<7)|(1<<6)|(1<<3));
	TCCR2B |= (1<<2)|(1<<1);

	unsigned char timer2;
	while(1){
		//timer2 = TCNT2;
		//PORTB = (TCNT2>>7);
	}
}

ISR(TIMER2_OVF_vect,ISR_BLOCK)
{
	PORTB ^= (1<<0);
}

