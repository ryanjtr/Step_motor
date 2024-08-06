/*This file has been prepared for Doxygen automatic documentation generation.*/
/*! \file *********************************************************************
 *
 * \brief Stepper motor driver.
 *
 * Stepper motor driver, increment/decrement the position and outputs the
 * correct signals to stepper motor.
 *
 * - File:               sm_driver.c
 * - Compiler:           IAR EWAAVR 4.11A
 * - Supported devices:  All devices with a 16 bit timer can be used.
 *                       The example is written for ATmega48
 * - AppNote:            AVR446 - Linear speed control of stepper motor
 *
 * \author               Atmel Corporation: http://www.atmel.com \n
 *                       Support email: avr@atmel.com
 *
 * $Name: RELEASE_1_0 $
 * $Revision: 1.2 $
 * $RCSfile: sm_driver.c,v $
 * $Date: 2006/05/08 12:25:58 $
 *****************************************************************************/

#include <avr/io.h>
#include "global.h"
#include "sm_driver.h"

//! Position of stepper motor (relative to starting position as zero)
int stepPosition = 0;

/*! \brief Init of io-pins for stepper motor.
 */
void sm_driver_Init_IO(void)
{
  // Init of IO pins
  DDRC |= ((1 << DDC2) | (1 << DDC3)); // Set output pin direction registers to output
}
