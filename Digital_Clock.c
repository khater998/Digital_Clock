/*
 * Digital_Clock.c
 *
 *  Created on: Mar 8, 2023
 *      Author: khater
 */

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#define sev_seg1 0b00000001
#define sev_seg2 0b00000010
#define sev_seg3 0b00000100
#define sev_seg4 0b00001000
#define sev_seg5 0b00010000
#define sev_seg6 0b00100000

void TIMER1_init(void);
void INT0_init(void);
void INT1_init(void);
void INT2_init(void);

unsigned char sec_p0, sec_p1, min_p0, min_p1, hr_p0, hr_p1;
/* sec_p0   =>   seconds position 0 || 1st 7SEG
 * sec_p1   =>   seconds position 1 || 2nd 7SEG
 * min_p0   =>   minutes position 0 || 3rd 7SEG
 * min_p1   =>   minutes position 1 || 4th 7SEG
 * hr_p0    =>   hours position 0	|| 5th 7SEG
 * hr_p1    =>   hours position 1	|| 6th 7SEG
 */

int main()
{
	/* Initialize the first four pin of portC as output,
	 * to be used as Decoder input.
	 */
	DDRC |= 0x0F;
	PORTC &= 0xF0;

	/* Initialize first 6 pins of portA as output,
	 * to be used as enable pins of the 6 seven segments
	 */
	DDRA |= 0x3F;
	PORTC &= 0xC0;

	/* Make PD2(INT0_pin) input pin && enable internal pull up*/
	DDRD &= ~(1 << PD2);
	PORTD |= (1 << PD2);

	/* Make PD3(INT1_pin) */
	DDRD &= ~(1<<PD3);

	/* Make PB2(INT0_pin) input pin && enable internal pull up*/
	DDRD &= ~(1 << PB2);
	PORTD |= (1 << PB2);

	TIMER1_init();
	INT0_init();
	INT1_init();
	INT2_init();



	while(1)
	{
		PORTA = (PORTA & 0xC0) | (sev_seg1);
		PORTC = (PORTC & 0xF0) | (sec_p0);
		_delay_ms(2);

		PORTA = (PORTA & 0xC0) | (sev_seg2);
		PORTC = (PORTC & 0xF0) | (sec_p1);
		_delay_ms(2);

		PORTA = (PORTA & 0xC0) | (sev_seg3);
		PORTC = (PORTC & 0xF0) | (min_p0);
		_delay_ms(2);

		PORTA = (PORTA & 0xC0) | (sev_seg4);
		PORTC = (PORTC & 0xF0) | (min_p1);
		_delay_ms(2);

		PORTA = (PORTA & 0xC0) | (sev_seg5);
		PORTC = (PORTC & 0xF0) | (hr_p0);
		_delay_ms(2);

		PORTA = (PORTA & 0xC0) | (sev_seg6);
		PORTC = (PORTC & 0xF0) | (hr_p1);
		_delay_ms(2);



	}
}

/*****************************************************
 *****************************************************/
ISR(TIMER1_COMPA_vect)
{

	sec_p0++; // increase seconds with every interrupt
	if(sec_p0 == 10) // if it's 10: reset && increase the next position
	{
		sec_p0 = 0;
		sec_p1++;
		if(sec_p1 == 6) // seconds resets at 60 && increment minutes
		{
			sec_p1 = 0;
			min_p0++;
			{
				if(min_p0 == 10) // if first position of minutes is 10: reset && increment next position
				{
					min_p0 = 0;
					min_p1++;
					if(min_p1 == 6) // reset at 60 && increment hours position
					{
						min_p1 = 0;
						hr_p0++;
						{
							/* check if 2nd position in hours is 2 && 1st position is 4:
							 * if true, reset the two positions
							 */
							if(hr_p0 == 4 && hr_p1 == 2)
							{
								hr_p0 = 0;
								hr_p1 = 0;
							}else if(hr_p0 == 10)
							{
								hr_p0 = 0;
								hr_p1++;
							}
						}
					}
				}
			}
		}
	}

}

/*****************************************************
 *****************************************************/
ISR(INT0_vect)  // reset button pressed
{
	/* RESET CLOCK */
	TCNT1 = 0;
	sec_p0 = 0;
	sec_p1 = 0;
	min_p0 = 0;
	min_p1 = 0;
	hr_p0 = 0;
	hr_p1 = 0;
}

/*****************************************************
 *****************************************************/
ISR(INT1_vect)  // stop button pressed
{
	/* STOP TIMER */
	TCCR1B &= (0xF8);
}

/*****************************************************
 *****************************************************/
ISR(INT2_vect) // resume button pressed
{
	/* RESUME TIMER */
	TCCR1B |= (1 << CS10) | (1 << CS12);
}

/*****************************************************
 *****************************************************/
void TIMER1_init(void)
{
	// 	Fcpu = 1Mhz, pre-scaler = 1024
	//  CTC MODE
	TCCR1A |= (1 << FOC1A);
	OCR1A = 976;
	TCCR1B |= (1 << CS10) | (1 << CS12) | (1 << WGM12);


	// enable timer 1 compare match interrupt
	TIMSK |= (1 << OCIE1A);
	SREG |= (1<<7);
}

/*****************************************************
 *****************************************************/
void INT0_init(void)
{
	/* 1. select trigger condition */
	MCUCR |= (1 << ISC01);

	/* 2. Enable interrupt 0 */
	GICR |= (1 << INT0);

}

/*****************************************************
 *****************************************************/
void INT1_init(void)
{
	/* 1. select trigger condition */
	MCUCR |= (1 << ISC10) |(1 << ISC11);

	/* 2. Enable interrupt 1 */
	GICR |= (1 << INT1);

}

/*****************************************************
 *****************************************************/
void INT2_init(void)
{
	/* 1. select trigger condition */
	MCUCSR |= (1 << ISC2);

	/* 2. Enable interrupt 2 */
	GICR |= (1 << INT2);

}

