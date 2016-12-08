#include <avr/io.h>
#include <avr/interrupt.h>

#define FOSC 1000000 // Clock Speed
#define BAUD 9600
#define MYUBRR FOSC/16/BAUD-1
#define MEAN 8
#define TOTSTEPS 96
#define CONV_TO_RPM 60*1000000*MEAN/(TOTSTEPS*64)
#define GET 255
#define HZ 15624


static volatile unsigned char speed;
static volatile unsigned int intCnt = 0;
static volatile unsigned char cycIx;
static volatile unsigned char tmp;

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

ISR(USART_RX_vect,ISR_BLOCK)
{
	/*Data received*/
	unsigned char data = UDR0;

	if(data == GET)
	{
		USART_Transmit((unsigned char) speed);
	}
	else
	{

	}
}

ISR(PCINT0_vect, ISR_BLOCK) //SPEED MEASURMENT
{
	//PORTB ^= 1;

	/*if((cycIx & 15) == 1)
	{
		//PORTB ^= 1;
		unsigned int temp = TCNT1;
		TCNT1 = 0;
		speed = CONV_TO_RPM/temp;
	}
	cycIx++;

	*/

	//PORTB ^= 1;
	
	if((PINB & 4) == 4){
		//PORTD ^= (1 << PD7);
		intCnt += 1;
		if (intCnt > 0)
			PORTD ^= (1 << PD7);
	}

	PCIFR |= (1 << PCIF0);
}

ISR(TIMER1_COMPA_vect, ISR_BLOCK)
{ 	
	static int intCnt = 0;
	//PORTD ^= (1 << PD7);
	intCnt +=1;
	if(intCnt ==1){
			PORTB ^= 1;
		}

	tmp = intCnt;
	//intCnt = 0;

	speed = tmp*60/(24.0);
}

void PWM_Init(void)
{	
	PRR &= ~(1 << PRTIM2); //Enable timer
	TIMSK2 |= (1 << OCIE1B)|(1 << OCIE1A)|(1 << TOIE1);
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
	OCR2B = 250; //Setting duty
}

void SM_Init(void)
{

	//Timer setup
	PRR &= ~(1 << PRTIM1); //Enable timer one
	TCCR1A = 0;
	TCCR1B = 0;
	TCCR1B |= (1 << CS11)|(1 << CS10)|(1 << WGM12); //Prescaler 64
	TCNT1H = 0;
	TIMSK1 |= (1 << OCIE1A);
	OCR1A = HZ;


	DDRB &= ~((1 << PB1)|(1 << PB2));
	PORTB |= (1 << PB1)|(1 << PB2);
	cli();
	PCMSK0 |= (1 << PCINT1); //Enable pin change interrupt
	PCICR |= (1 << PCIE0); //Enable interrupt
	sei();

}

int main(void)
{
	
	//intCnt = (int*)malloc(1);
	
	
	//PORTB = (1 << 0); //DEBUGLED2
	//USART_Init(MYUBRR);
	sei();
	//UCSR0B |= 0x90; /*Enable receive interrupt*/
	PWM_Init();
	SM_Init();


	while(1){

		//PORTB = (intCnt & 1);
		/*if(intCnt > 0){
			PORTB &= ~1;
		}else{
			PORTB |= 1;
		}*/

	}
	return 0;

}
