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
/*#include <util/delay.h>*/


//! Global status flags
struct GLOBAL_FLAGS status = {FALSE, FALSE, 0};

void ShowHelp(void);
void ShowData(int position, int acceleration, int deceleration, int speed, int steps);

/*! \brief Init of peripheral devices.
 *
 *  Setup IO, uart, stepper, timer and interrupt.
 */
void Init(void)
{
  // Init of IO pins
  sm_driver_Init_IO();

  // Set stepper motor driver output
/*  sm_driver_StepOutput(0);*/

  // Init of Timer/Counter1
  speed_cntr_Init_Timer0();

  sei();
}
void main(void)
{
	// Number of steps to move.
	int steps = 2000;
	// Accelration to use.
	int acceleration = 700;
	// Deceleration to use.
	int deceleration = 200;
	// Speed to use.
	int speed = 800;


	Init();
	
	while(1)
	{
		speed_cntr_Move(steps, acceleration, deceleration, speed);
		__asm__ __volatile__("nop");

		
	}
	
	
}

