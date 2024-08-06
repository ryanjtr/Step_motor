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
//#include "uart.h"


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
	//// Init Uart
	//usart0_init();
	sei();
}
void main(void)
{
	// Number of steps to move.
	int steps = 10000;
	// Accelration to use.
	int acceleration = 1500;
	// Deceleration to use.
	int deceleration = 1500;
	// Speed to use.
	int speed = 15000;

	Init();

	speed_cntr_Move(steps, acceleration, deceleration, speed);

	while (1)
	{
		
	}
}
