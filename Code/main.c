/*
 * main.c
 *
 *  Created on: Sep 10, 2024
 *      Author: Administrator
 */
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#define F_CPU 16000000
unsigned char i;
unsigned char seconds_counter_ones=0;
unsigned char seconds_counter_tenth=0;
unsigned char minutes_counter_ones=0;
unsigned char minutes_counter_tenth=0;
unsigned char hours_counter_ones=0;
unsigned char hours_counter_tenth=0;
unsigned char toggle_mode=0;


void ports_init(void){ //all used ports' configurations
	DDRC|=0x0F;    //PORTC as output for 7 segment display
	DDRA|=0x3F;   //PORTA as output for enable/disable of the multiplexed mode 6 7-Segments
	DDRD&=~(1<<DDD2) & (1<<DDD3); //PD2 and PD3 as input for reset button, pause button
	PORTD|=(1<<PORTD2); //PD2 as input pull resistor
	DDRD|=(1<<DDD4)|(1<<DDD5)|(1<<DDD0); //Output pins for count up/down LEDs
	DDRB=0x00;      //PORTB as input
	PORTB=0xFF;     //Internal Pull-Up resistor
	MCUCR|=(1<<ISC01)|(1<<ISC11);  //Enabling INT0 and INT1 with falling edge triggering
	GICR|=(1<<INT0)|(1<<INT1)|(1<<INT2);    //INT0, INT1 and INT2 Request Enable
	MCUCSR|=(1<<ISC2);
}
ISR(INT0_vect){  //Interrupt for RESET feature
	seconds_counter_ones=0;
	seconds_counter_tenth=0;
	minutes_counter_ones=0;          //All segments are off
	minutes_counter_tenth=0;
	hours_counter_ones=0;
	hours_counter_tenth=0;
	toggle_mode=0;                 //back to count up mode
	PORTD&=~(1<<PD0);             //turn off buzzer
}
ISR(INT1_vect){ //Interrupt to pause feature
	TCCR1B&=~(1<<CS12); //turn off pre-scale (stop timer)
}
ISR(INT2_vect){  //Interrupt to resume feature
	TCCR1B|=(1<<CS12);  //pre-scale (timer begins)
}
void timer1_init(void){ //Configuration of timer1 registers
	TCNT1=0;   //ensuring the counter is empty
	OCR1A=62500;    //compare value to achieve the desired time
	TCCR1A|=(1<<WGM12); //Timer1 in CTC mode
	TIMSK|=(1<<OCIE1A); //Timer Interrupt MASK enabled to have an ISR at compare match
	TCCR1B|=(1<<CS12);  //Pre-scale with N=256 and timer begins
}
ISR(TIMER1_COMPA_vect){
	if(toggle_mode==0){
		increment_timer();  //Timer increment scenario
	}
	else{
		decrement_timer();   //Timer decrement scenario
	}
}
void decrement_timer(void) {
	PORTD|=(1<<PD5);     //turn on count down led
	PORTD&=~(1<<PD4);    //turn off count up led
	seconds_counter_ones--;
	if (seconds_counter_ones == 255) {
		seconds_counter_ones = 9;
		seconds_counter_tenth--;
	}
	if (seconds_counter_tenth == 255) {
		seconds_counter_tenth = 5;
		minutes_counter_ones--;
	}
	if (minutes_counter_ones == 255) {
		minutes_counter_ones = 9;
		minutes_counter_tenth--;
	}
	if (minutes_counter_tenth == 255) {
		minutes_counter_tenth = 5;
		hours_counter_ones--;
	}
	if (hours_counter_ones == 255) {
		hours_counter_ones = 9;
		hours_counter_tenth--;
	}
	if (hours_counter_tenth == 255) {
		hours_counter_tenth = 1;
	}
	if(seconds_counter_ones==0 && seconds_counter_tenth==0 && minutes_counter_ones==0 && minutes_counter_tenth==0 && hours_counter_ones==0 && hours_counter_tenth==0){
		timer_end();
	}
}
void increment_timer(void){
	PORTD|=(1<<PD4);    //turn on count up led
	PORTD&=~(1<<PD5);  //turn off count down led
	seconds_counter_ones++;
	if(seconds_counter_ones == 10){
		seconds_counter_ones = 0;
		seconds_counter_tenth++;
	}
	if(seconds_counter_tenth == 6){
		seconds_counter_tenth = 0;
		minutes_counter_ones++;
	}
	if(minutes_counter_ones == 10){
		minutes_counter_ones = 0;
		minutes_counter_tenth++;
	}
	if(minutes_counter_tenth == 6){
		minutes_counter_tenth = 0;
		hours_counter_ones++;
	}
	if(hours_counter_ones == 10){
		hours_counter_ones = 0;
		hours_counter_tenth++;
	}
	if(hours_counter_tenth == 2 && hours_counter_ones == 4){
		hours_counter_tenth = 0;
		hours_counter_ones = 0;
	}
}
void timer_end(void){
	TCCR1B&=~(1<<CS12);   //timer stops
	PORTD&=~(1<<PD5);     //turn off countdown led
	PORTD|=(1<<PD0);       //turn on buzzer
}
void seven_segment_display(void){ //displaying seconds, minutes, hours in multiplexing mode
	for (i = 0; i < 6; i++) {
		PORTA = (1 << i); // Select the segment to light up
		switch(i) {
		case 0:
			PORTC = (hours_counter_tenth); // Display minutes ones
			break;
		case 1:
			PORTC = (hours_counter_ones); // Display minutes tenth
			break;
		case 2:
			PORTC = (minutes_counter_tenth); // Display hours ones
			break;
		case 3:
			PORTC = (minutes_counter_ones); // Display hours tenth
			break;
		case 4:
			PORTC = (seconds_counter_tenth); // Display seconds ones
			break;
		case 5:
			PORTC = (seconds_counter_ones); // Display seconds tenth
			break;
		}
		_delay_ms(5); // Small delay for multiplexing
		PORTA &= ~(1 << i); // Turn off the current segment
	}
}
uint8_t button_pressed(uint8_t pin)
{
	if (!(PINB & (1 << pin))) {  // Check if the button is pressed (pin LOW)
	   _delay_ms(30);           // De-bounce delay
		if (!(PINB & (1 << pin))) {  // Confirm button is still pressed
			return 1;  // Button is pressed
		}
	}
	return 0 ;  // Button is not pressed
}




// Adjust countdown time: Hours, Minutes, Seconds
void adjust_time()
{

	//MINUTES INCREMENT
	if(button_pressed(PB4))
	{
		_delay_ms(50);
		if(button_pressed(PB4))
		{
			minutes_counter_ones++;
			if(minutes_counter_ones==10)
			{
				minutes_counter_ones=0;
				minutes_counter_tenth++;
				if (minutes_counter_tenth==6)
				{
					minutes_counter_tenth=0;

				}
			}
			_delay_ms(200);

		}
	}
	//MINUTES DECREMENT
	if (button_pressed(PB3))
	{
		_delay_ms(30);
		if (button_pressed(PB3))
		{
			if(minutes_counter_ones==0)
			{
				minutes_counter_ones=9;
				if(minutes_counter_tenth==0)
				{
					minutes_counter_tenth=5;
				}
				else
				{
					minutes_counter_tenth--;
				}
			}
			else
			{
				minutes_counter_ones--;
			}
		}
		_delay_ms(200);
	}
	//SECONDS INCREMENT
					if (button_pressed(PB6))
					{
						_delay_ms(30);
						if(button_pressed(PB6))
						{
							seconds_counter_ones++;
							if(seconds_counter_ones==10)
							{
								seconds_counter_ones=0;
								seconds_counter_tenth++;
								if(seconds_counter_tenth==6)
								{
									seconds_counter_tenth=0;
								}
							}

						}
						_delay_ms(200);

					}
					//SECONDS DECREMENT
					if (button_pressed(PB5))
							{
						_delay_ms(30);
						if (button_pressed(PB5))
						{
							if (seconds_counter_ones == 0) {
							        seconds_counter_ones = 9;
							        if (seconds_counter_tenth == 0)
							        {
							            seconds_counter_tenth = 5;  // Set to 5 to allow wrap from 00 to 59
							        } else
							        {
							            seconds_counter_tenth--;
							        }
							    }
							else
							    {
							        seconds_counter_ones--;
							    }
							_delay_ms(200);
						}

							}
}
void adjust_hours(void){
	//HOURS INCREMENT
	if (!(PINB & (1 << PB1))) {  // Check if the button is pressed (pin LOW)
		   _delay_ms(30);           // De-bounce delay
			if (!(PINB & (1 << PB1))) {
				hours_counter_ones++;
				if(hours_counter_ones==10)
				{
					hours_counter_ones=0;
					hours_counter_tenth++;
					if(hours_counter_ones==4 && hours_counter_tenth==2)
					{
						hours_counter_tenth = 0;
						hours_counter_ones = 0;
					}

				}
			}
			_delay_ms(200);

		}
		//HOURS DECREMENT
		if (button_pressed(PB0))
		{
			_delay_ms(30);
			if (button_pressed(PB0))
			{
				if(hours_counter_ones==0 && hours_counter_tenth==0 )
				{
					hours_counter_ones=9;
					hours_counter_tenth=5;

				}
				else
				{

					hours_counter_ones--;
				}
			}
			else
			{
				hours_counter_tenth--;
			}


			_delay_ms(200);

		}
}
void toggle_check(void){
	if(!(PINB & (1 << PB7))){
				_delay_ms(30);
				if(!(PINB & (1 << PB7))){
					toggle_mode=!toggle_mode;
				}
			}
}
int main(){
	sei();               //Enabling the global interrupt
	ports_init();
	timer1_init();
	while(1)
	{
		seven_segment_display();
		adjust_time();
		adjust_hours();
		toggle_check();
	}


}


