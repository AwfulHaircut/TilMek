#include <avr/io.h>
#include<avr/interrupt.h>
#define FOSC 1000000 // Clock Speed
#define BAUD 9600
#define MYUBRR FOSC/16/BAUD-1
#define MEAN 8
#define TOTSTEPS 96
#define CONV_TO_RPM 60*1000000*MEAN/(TOTSTEPS*64)

volatile unsigned int speed;
volatile unsigned char cycIx = 0;

void USART_Init(unsigned int ubrr)
{
	/*Set baud rate */
	UBRR0H = (unsigned char)(ubrr>>8);
	UBRR0L = (unsigned char)ubrr;
	/*Enable receiver and transmitter*/
	UCSR0B = (1<<RXEN0)|(1<<TXEN0);
	/* Set frame format: 8data, 2stop bit */
	UCSR0C = (1<<USBS0)|(3<<UCSZ00);
}

void USART_Transmit( unsigned char data )
{
	/* Wait for empty transmit buffer */
	while ( !( UCSR0A & (1<<UDRE0)) )
		;
	/* Put data into buffer, sends the data */
	UDR0 = data;
}

unsigned char USART_Receive( void )
{
	/* Wait for data to be received */
	while ( !(UCSR0A & (1<<RXC0)) )
		;
	/* Get and return received data from buffer */
	return UDR0;
}

ISR(USART_RX_vect,ISR_BLOCK)
{
	/*Data received*/
	char temp = USART_Receive();
	USART_Transmit(temp);
}

ISR(PCINT0_vect,ISR_BLOCK) //SPEED MEASURMENT
{
	PORTB ^= 1;

	if(cycIx >= 8)
	{
		unsigned int temp = TCNT1;
		TCNT1 = 0;
		speed = CONV_TO_RPM/temp;
		cycIx = 0;
	}
	else
	{
		cycIx = cycIx + 1;
	}

	PCIFR |= (1 << PCIF0);
}

void PWM_Init(void)
{	
	PRR &= ~(1 << PRTIM2); //Enable timer
	TIMSK2 |= (1<<2)|(1<<1)|(1<<0);
	DDRD |= (1 << 3); //Set pin output
	//TCCR0A &= ~0xF3; //PWM setup
	
	TCCR0A = 0;
	
	//TCCR2A &= ~((1 << 7)|(1 << 6)|(1 << 5)|(1 << 4)|(1 << 2)|(1 << 0));
	//TCCR0A |= 0x32;

	TCCR2A |= (1 << COM0B1)/*|(1 << COM0B0)*/; //Inverted Fast PWM
	TCCR2A |= (1 << WGM01)|(1 << WGM00); //MIN-MAX

	TCCR2B = 0;
	
	//TCCR0B &= ~0xCF; //PWM setup
	//TCCR2B &= ~((1 << 7)|(1 << 6)|(1 << 3)|(1 << 2)|(1 << 1)|(1 << 0));
	//TCCR0B |= 0x9;
	TCCR2B |= (1 << WGM22); //MIN-MAX
	TCCR2B |= (1 << CS20); //Prescaler N=1
	
	OCR2A = 250; //Setting TOP
	OCR2B = 240; //Setting duty
}

void SM_Init(void)
{
	//Timer setup
	PRR &= ~(1 << PRTIM1); //Enable timer one
	TCCR1A = 0;
	TCCR1B = 0;
	TCCR1B |= (1 << CS11)|(1 << CS10); //Prescaler 64
	TCNT1 = 0;


	DDRB &= ~(1 << PB1);
	PORTB |= (1 << PB1);
	cli();
	PCMSK0 |= (1 << PCINT1); //Enable pin change interrupt
	PCICR |= (1 << PCIE0); //Enable interrupt
	sei();

}

void send(unsigned char)
{

}

int main(void)
{
	//PORTB = (1 << 0); //DEBUGLED2
	//USART_Init(MYUBRR);
	sei();
	//UCSR0B |= 0x90; /*Enable receive interrupt*/
	PWM_Init();
	SM_Init();
	while(1){

		if(cycIx > 190){
			PORTB |= (1<<0);
		}
		
	}
	return 0;

}
