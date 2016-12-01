#define LED_LEFT	(1 << PB0)
#define LED_RIGHT	(1 << PD7)
#define PIN_INT1	(1 << PB1)
#define PIN_INT2	(1 << PB2)
#define POWER_PWM	(1 << PD3)
#define CONV_RPM 39063
#define CLK_FREQ 1000000
#define READ_PERIOD 1
#define CTRL_PERIOD 90
#define MEAN 8

#include <avr/io.h>
#include <avr/interrupt.h>

static volatile int speed;
static volatile int tmp;
static volatile int times[MEAN];
static volatile unsigned char arrayPos;

//Controll variables

volatile unsigned char u = 0;
volatile unsigned int ref = 40;

volatile double K = 1.4;
volatile double I = 0;
volatile double Ti = 0.6;
volatile double Tr = 0.6;
volatile double h = 1.0 / (CLK_FREQ / 1024 / CTRL_PERIOD); 

unsigned char saturate(signed int in)
{
	if(in > 255)
		return 255;
	if(in < 0)
		return 0;
	return in;
}

void io_init(void)
{
	DDRB	= LED_LEFT;
	DDRD	= LED_RIGHT | POWER_PWM;
	PORTB	= PIN_INT1;// | PIN_INT2;
}

void interrupt_init(void)
{
	PCICR	= (1 << PCIE0);
	PCMSK0	= (1 << PCINT1);//|(1 << PCINT2);
}

void timer_init(void)
{
	//Timer 1 init
	TCCR1B	|= (1 << CS01)|(1 << CS00);
	TIMSK1 |= (1 << TOIE1);

	//Timer 0 init
	TCCR0A |= (1 << WGM01);
	TCCR0B |= (1 << CS02) | (1 << CS00);
	TIMSK0 |= (1 << OCIE0A);
	OCR0A = CTRL_PERIOD;

}

void pwm_init(void)
{
	TCCR2A |= (1 << COM0B1);
	TCCR2A |= (1 << WGM01)|(1 << WGM00);
	TCCR2B |= (1 << CS20);

	OCR2B = 255; //DUTY

}

void set_pwm(char input)
{
	OCR2B = (input ^ 0xff);
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

    	/*if(speed > 100 && speed < 110)
		{
			PORTB |= LED_LEFT;
		}
		else
		{
			PORTB &= ~LED_LEFT;
		}*/

    }
}

ISR(PCINT0_vect)
{
	//PORTB ^= LED_LEFT;

	if((PINB & 4) > 0)
	{

		//PORTD ^= LED_RIGHT;

		times[arrayPos & (MEAN - 1)] = TCNT1;
		TCNT1 = 0;

		if((arrayPos & READ_PERIOD) == 1)
		{
			//PORTB ^= LED_LEFT;


			tmp = 0;

			for(int x = 0; x < MEAN; x = x + 1)
			{
				tmp = tmp + times[x];
			}

			tmp = tmp >> 3;
			speed = 1.0 * CONV_RPM / tmp - 2;

		}

		arrayPos = arrayPos + 1;
	}
}

ISR(TIMER0_COMPA_vect)
{
	//PORTB ^= LED_LEFT;

	signed int error = ref - speed;
	signed int v = I + K * error;
	u = saturate(v);

	set_pwm(u);
	
	I = I + (K * h / Ti) * error + (h / Tr) * (u - v);
}

ISR(TIMER1_OVF_vect)
{
	speed = 0;
}