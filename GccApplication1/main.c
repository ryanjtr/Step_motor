/*
 * GccApplication1.c
 *
 * Created: 31/07/2024 1:32:53 CH
 * Author : Asus
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include "sm_driver.h"
#include "speed_cntr.h"
#include "global.h"
#include "uart.h"
/*#include <util/delay.h>*/
int64_t str1[30];
int64_t str2[100];
int64_t str3[30];

#define T1_FREQ_ 1000000
#define SPR_ 200
#define ALPHA_ (2*3.14159/SPR_)
#define A_T_x100_ ((long)(ALPHA_*T1_FREQ_*100))
#define T1_FREQ_148_ ((int)((T1_FREQ_*0.676)/100)) // divided by 100 and scaled by 0.676
#define A_SQ_ (long)(ALPHA_*2*10000000000)         // ALPHA*2*10000000000
#define A_x20000_ (int)(ALPHA_*20000)

unsigned long sqrt_t_(unsigned long x)
{
	register unsigned long xr; // result register
	register unsigned long q2; // scan-bit register
	register unsigned char f;  // flag (one bit)

	xr = 0;           // clear result
	q2 = 0x40000000L; // higest possible result bit
	do
	{
		if ((xr + q2) <= x)
		{
			x -= xr + q2;
			f = 1; // set flag
		}
		else
		{
			f = 0; // clear flag
		}
		xr >>= 1;
		if (f)
		{
			xr += q2; // test flag
		}
	} while (q2 >>= 2); // shift twice
	if (xr < x)
	{
		return xr + 1; // add for rounding
	}
	else
	{
		return xr;
	}
}

speedRampData srd;
// //! Global status flags
struct GLOBAL_FLAGS status = {FALSE, FALSE, 0};

// void ShowHelp(void);
// void ShowData(int position, int acceleration, int deceleration, int speed, int steps);

/*! \brief Init of peripheral devices.
 *
 *  Setup IO, uart, stepper, timer and interrupt.
 */
void Init(void)
{
  // Init of IO pins
  sm_driver_Init_IO();

  // Init of Timer/Counter1
  speed_cntr_Init_Timer1();	
	// Init Uart
	usart0_init();
   sei();
}
void main(void)
{
  // Number of steps to move.
  int steps = 200;
  // Accelration to use.
  int acceleration = 50;
  // Deceleration to use.
  int deceleration = 50;
  // Speed to use.
  int speed = 100;
	
// 	unsigned int accel_ = 25;
// 	unsigned int speed_ = 50;
// 	int64_t min_delay = 32000;//A_T_x100_ / speed_;
// 	printf("min delay= %d\n",min_delay);
// 	unsigned int step_delay = (T1_FREQ_148_ * sqrt_t_(A_SQ_ / accel_)) / 100;
// 	printf("step delay= %d\n",step_delay);
// 
// 
	Init();

// 	for(int64_t num = min_delay;num<40000;num+=100)
// 	{
// 			usart0_send_string("step d= ");
// 			sprintf(str2,"%d",num);
// 			usart0_send_string(str2);
//  			usart0_send_string("\n");
// 	}

// 	usart0_send_string("min d= ");
// 	sprintf(str3,"%d",min_delay);
// 	usart0_send_string(str3);
// 	usart0_send_string("\n");

	while(1)
	{
	
speed_cntr_Move(steps, acceleration, deceleration, speed);
// 	usart0_send_string("status= ");
// 	sprintf(str2,"%d",srd.run_state);
// 	usart0_send_string(str2);
// 	usart0_send_string("\n");
// 	
// 
 	}
	
	
}
