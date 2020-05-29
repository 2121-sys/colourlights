
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#define F_CPU 1000000UL

// initialise variables

//used in task 3
volatile int switch_check = 0;
uint8_t LB_counter_sw5=0;
uint8_t LB_counter_sw6=0;
volatile double distance =0; 
volatile double speed=0;
volatile uint32_t LB1_time=0;
volatile uint32_t LB2_time=0;
volatile uint32_t t_readingLB1=0;
volatile uint32_t t_readingLB2=0;
volatile uint32_t time_difference_swich=0;
volatile double time_difference_secounds=0.0;
volatile uint32_t No_LB_counter_overflow=0;
volatile uint32_t No_ticks=0;
volatile uint32_t No_overflow_ticks=0;
volatile int task3_pwm = 0; 

//task 4
volatile int redon=0;
volatile int counter_task4=0;
volatile int flash_start=0;
volatile int LED3_overflowcounter=0;
volatile double no_cars=0;
volatile int task4_pwm = 1; 

//task 1 & 2
uint8_t mode = 0;
uint8_t lightcounter = 0;
volatile int config=0;
volatile int switchconfig_pressed = 0;
uint8_t period=1;
int LEDcounter=0;
int check=0;
uint16_t adc_value =0;


// ISR of timer 2 should take 0.5s
/*This timer overflow is used in task 4, to light up the LED3
When the swiched is pressed, it will go to the (flash_start=1) if loop.  
The LED3 will light on and off starting when the "counter_task4" variable is 0, 245
490, and 735 to response the LED3 within 10ms
The overflow counter variable increment at end of if loops and is being rest to 0 
only when the which is pressed (when interrupt 0 occurs) 
 */

ISR(TIMER2_OVF_vect){
	if(flash_start==1){
		
		
		
		if(counter_task4 == 245*0){
			PORTB &= ~(1<<PB5);
		}
		
		else if(counter_task4 == (245*1)){
			PORTB|=(1<<PB5);
		}
		
		else if(counter_task4 == (245*2)){
			PORTB &= ~(1<<PB5);
		}
		
		else if(counter_task4 == (245*3) ){
			//counter_task4=0;
			PORTB|=(1<<PB5);
			flash_start=0;
		}
		counter_task4++;
		
	}
		
		
	TCNT2=0;
	
}

// traffic light normal and configuration mode
ISR(TIMER1_OVF_vect) {
	LB_counter_sw5++;
	
	//TASK 1
	/*lightcounter is the variable that changes to 0,1 and 2 
	as colour lights changers from red, green and yellow. 
	And when the corrosponding colour is turned on the timer 1
	reset overflow in 1 second and increment "lightcounter" variable */
	if (lightcounter==0){
		//red
		redon=1;
		PORTB |= (1<<PB1); //turn yellow off
		PORTB &= ~(1<<PB0); //turn red on
		lightcounter++;     
		TCNT1=(0xFFFF-(15625*period));  
		//---TASK2---//
		/*enter to the configuration mode which the switch id pressed. 
		Make the lightcounter variable to zero, when the red is on. 
		Have a LEDcounter variable where it will increment upto 2 if the period is 2. 
		once the LEDcounter increment to 2(when the period is 1), it will exit from that loop.*/
		// ---- Configuration mode start ---- //
		if (switchconfig_pressed==1){
			lightcounter=0;
			potentiometer();

			if (LEDcounter < (period*2)){
				LEDcounter++;
				potentiometer();
				PORTB^=(1<<PB4);
				TCNT1=57722; //0.5s
			}
			else{
				potentiometer();
				PORTB|=(1<<PB4);
				LEDcounter = 0;
				TCNT1=(0xFFFF-(15625*period));
			}
		}
		
		else if(switchconfig_pressed==0){
			if ( config==1){
				PORTB&= ~(1<<PB4);
				config==0;
			}
			TCNT1=(0xFFFF-(15625*period));
		}
		// ---- Configuration mode end ---- //
	}	
	
	else if (lightcounter==1 ){
		//green
		redon=0;
		PORTB |= (1<<PB0); //turn red off
		PORTB &= ~(1<<PB2);	//turn green on
		lightcounter++;
		TCNT1=(0xFFFF-(15625*period)); 
	}

	else if(lightcounter==2 ){
		//yellow
		redon=0;
		PORTB |= (1<<PB2); //turn green off
		PORTB &= ~(1<<PB1); //turn yellow on
		lightcounter=0;
		TCNT1=(0xFFFF-(15625*period));
	}
	
}

/*the potentiometer is a function that uses the ADC values and
 changes the period accordingly*/
void potentiometer(){
	ADCSRA |= (1<<ADSC); //start conversion mode
	adc_value=ADC;
	//ADIF-interrupt flag activated. flag reset at end of conversion 
	if (ADCSRA & (1<<ADIF)) 
	{
		// ADC input read
		adc_value=ADC;
		// convert input to step level
		if (adc_value < 256)
		{
			period = 1;
		}
		else if (adc_value < (256*2))
		{
			period = 2;
		}
		else if (adc_value < (256*3))
		{
			period = 3;
		}
		else if (adc_value < (256*4))
		{
			period = 4;
		}
		ADCSRA |= (1<<ADIF); 
	}
}


// task4 - red light camera. camera LED should flash when SW7 is pressed
/*This is the interrupt for task 4. The "redon" is a variable that equals to 1
when it's on red. So when the button is pressed when it's red. it will set the 
flash_start=1, counter_task4=0 which used in the timer 2, and increment the 
number of cars (no_cars).
The output of PWM is done in this interrupt*/
ISR(INT0_vect){
	
	if(redon==1){ 
		flash_start=1;
		counter_task4=0;
		no_cars++; 
	}
	// output PWM for task 4
	OCR2=((uint16_t)(((no_cars)*(1))*(255.0/100.0)));
}

// task 2 - config button. 
ISR(INT1_vect){
	switchconfig_pressed=!switchconfig_pressed;
}


int main(void)
{
	//task 1
	DDRB |= (1<<DDB0); //red
	DDRB |= (1<<DDB1); //yellow
	DDRB |= (1<<DDB2); //green
		
	//task 2
	DDRB |= (1<<DDB4);//led4
		
	//task 3
	DDRD &= ~(1<<DDD5); //LB1
	DDRD &= ~(1<<DDD6); //LB2
	DDRB |= (1<<DDB3); //PWM
	PORTD |= (1<<PD5); //SW5 turn off
	PORTD |= (1<<PD6); //SW6 turn off
	// debug LEDs
	DDRB |= (1<<DDB7); //LED5
	DDRB |= (1<<DDB6); // LED6
		
	//task 4
	DDRB |= (1<<DDB5); //LED3

	//pwm->task 3
	DDRD &= ~(1<<DDD0); 
	
	//pwm->task 4
	DDRD &= ~(1<<DDD1);
	
	
	//initialise LED & switches
	PORTB |= 0xff; 
	PORTD |= (1<<PD5); // lb1
	
	//for ADC 
	
	ADCSRA |= (1<<ADEN); //enable mode
	ADCSRA |= (1<<ADPS1)|(1<<ADPS0);//prescale to 8
	ADCSRA &= ~(1<<ADPS2);
		
	ADMUX&= ~(1<<MUX0); // set multiplexor
	ADMUX&= ~(1<<MUX1);
	ADMUX&= ~(1<<MUX2);
	ADMUX&= ~(1<<MUX3);
		
	ADCSRA |= (1<<ADSC); //start conversion mode
	ADCSRA |= (1<<ADFR); //free running mode
	
	
	
	//for timer 1
	//timer 1 setting up
	TCCR1A &= ~(1<<WGM10);
	TCCR1A &= ~(1<<WGM11);
	TCCR1B &= ~(1<<WGM12);
	TCCR1B &= ~(1<<WGM13);
	
	//prescale to 64 for timer 1
	TCCR1B &= ~(1<<CS12);
	TCCR1B |= (1<<CS11);
	TCCR1B |= (1<<CS10);
	
	//for timer 2-fast pwm (waveform generation mode)
	TCCR2 |= (1<<WGM21);
	TCCR2 |= (1<<WGM20);
	
    //compare output non inverting
    TCCR2 &= ~(1<<COM20);
    TCCR2 |= (1<<COM21);

	//prescale 8 of timer 2->check to above if what to
	TCCR2 &= ~(1<<CS20);
	TCCR2 |= (1<<CS21);
	TCCR2 &= ~(1<<CS22);

	//timer0
	TCCR0 |= (1<<CS00);
	TCCR0 &=~(1<<CS01);
	TCCR0 |= (1<<CS02);
	

   // enable timer1 overflow interrupt
	TIMSK |= (1<<TOIE1);
	//
	//enable timer2 overflow interrupt
	TIMSK |=(1<<TOIE2);
	
	//task4
	GICR |= (1 << INT0);      // Turns on INT0 => PD2
	//intterupt 0 sense control for falling edge
	MCUCR &= ~(1<<ISC00);
	MCUCR |= (1<<ISC01); 
	
	//task2
	GICR |= (1 << INT1);     // Turns on INT1 => PD3
	//intterupt 1 sense control for falling edge
	MCUCR &= ~(1<<ISC10);
	MCUCR |= (1<<ISC11);
	
	PORTB |= 0xff;
	sei();
	TCNT1=(0xFFFF-(15625*period));
	TCNT2=0;	

	
	while (1)
	{
		// toggle pwm => task 3
		if (!((PIND &(1<<PD0))>>PD0)) { 
			OCR2=((uint16_t)(speed*(255.0/100.0)));
		}
		
		// toggle pwm => task 4
		if (!((PIND &(1<<PD1))>>PD1)) {
			OCR2=((uint16_t)(((no_cars)*(1))*(255.0/100.0)));
		}

		// ---- task 3 - speed monitor start ---- //
		
		/*We used polling for this task. The "switch_check" is a variable that makes sure the switch 5 
		doesn't response if the which 6 is pressed before hand.
		When the switches are pressed it will record the time in "LB1_time" variable
		and "LB2_time" variable and record the number of overflow in "No_LB_counter_overflow" 
		variable		
		The time is calculated by finding the difference of ticks when the switch 5 and 6 is pressed
		and sum it with, calculating the no of overflow ticks in a second. Then figure out the time difference by 
		dividing no of total tics by no of ticks takes for a second.
		The varaibles that used for calculate the time is in unit32, to make sure all the data is saved in the 
		variables.
		*/
		if((!((PIND &(1<<PD5))>>PD5))&& switch_check ==0){
			//SW5 pressed. LB1.
			LB1_time=TCNT1;
			LB_counter_sw5=0;
			switch_check =1;
		}
		
		if((!((PIND &(1<<PD6))>>PD6))&&switch_check==1){
			//SW6 pressed. LB2.
			LB2_time=TCNT1;
			No_LB_counter_overflow=LB_counter_sw5;
			// reset LB1 check variable
			switch_check=0;
			// recording time
			t_readingLB1=LB1_time; 
			t_readingLB2=LB2_time;
			//calculate the time 
			time_difference_swich=t_readingLB2-t_readingLB1;
			No_overflow_ticks=(No_LB_counter_overflow*15625*period); //make it 15625*period
			No_ticks=time_difference_swich+No_overflow_ticks;
			time_difference_secounds=((double)No_ticks)/15625.0;
			distance=20.0;
			// calculate speed
			speed=(distance/time_difference_secounds)*(3.6);
			if (speed > 100){
				speed=100;
			}
			// output pwm 			 
			OCR2=((uint16_t)(speed*(255.0/100.0))); //multiple by 255 because that is the no of bits in timer 2
		}
		// ---- task 3 - speed monitor end ---- //
	}
}

