#define LED_LEFT	(1 << PB0)
#define LED_RIGHT	(1 << PD7)
#define PIN_INT1	(1 << PB1)
#define PIN_INT2	(1 << PB2)
#define POWER_PWM	(1 << PD3)
#define CONV_RPM 7268
#define CLK_FREQ 1000000
#define READ_PERIOD 1
#define CTRL_PERIOD 90

#include <avr/io.h>
#include <avr/interrupt.h>

static volatile int speed;
static volatile int tmp;
static volatile int times[8];
static volatile char arrayPos;

//Controll variables

volatile unsigned char u = 39;
volatile unsigned int ref = 90;

volatile double K = 1;
volatile double I = 0;
volatile double Ti = 1;
volatile double Tr = 1;
volatile double h = 1.0 / (CLK_FREQ / 1024 / CTRL_PERIOD); 

unsigned char saturate(signed int in)
{
	if(in > 255)
		return 255;
	if(in < 0)
		return 0;
	return (in & 0xff);
}

void io_init(void)
{
	DDRB	= LED_LEFT;
	DDRD	= LED_RIGHT | POWER_PWM;
	PORTB	= PIN_INT1 | PIN_INT2;
}

void interrupt_init(void)
{
	PCICR	= (1 << PCIE0);
	PCMSK0	= (1 << PCINT1)|(1 << PCINT2);
}

void timer_init(void)
{
	//Timer 1 init
	TCCR1B	|= (1 << CS02)|(1 << CS01);

	//Timer 0 init
	TCCR2A |= (1 << WGM01);
	TCCR2B |= (1 << CS02) | (1 << CS00);
	TIMSK0 |= (1 << OCIEA);
	OCR0A = CTRL_PERIOD;

}

void pwm_init(void)
{
	TCCR2A |= (1 << COM0B1);
	TCCR2A |= (1 << WGM01)|(1 << WGM00);
	TCCR2B |= (1 << CS20);

	OCR2B = 255; //DUTY

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
    	if(speed > 15)
    		PORTB |= LED_LEFT;
    }
}

ISR(PCINT0_vect)
{

	times[arrayPos & 7] = TCNT0;
	TCNT0 = 0;

	if((arrayPos & READ_PERIOD) == 1)
	{
		tmp = 0;

		for(char x = 0; x < 8; x++)
		{
			tmp = tmp + times[x];
		}

		tmp >> 3;
		speed = CONV_RPM/tmp;

	}
}

ISR(TIMER0_COMPA_vect)
{
	signed int error = ref - speed;
	signed int v = I + K * error;
	u = saturate(v);
	//set duty with u

	//Calculate integral for next round
	I = I + (K * h / Ti) * error + (h / Tr) * (u - v);
}