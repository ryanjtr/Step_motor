/*This file has been prepared for Doxygen automatic documentation generation.*/
/*! \file *********************************************************************
 *
 * \brief Header file for sm_driver.c.
 *
 * - File:               sm_driver.h
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
 * $RCSfile: sm_driver.h,v $
 * $Date: 2006/05/08 12:25:58 $
 *****************************************************************************/

#ifndef SM_DRIVER_H
#define SM_DRIVER_H

// Direction of stepper motor movement
#define CW 0
#define CCW 1

/*! \Brief Define stepping mode to use in stepper motor.
 *
 * Either halfsteps (HALFSTEPS) or fullsteps (FULLSTEPS) are allowed.
 *
 */
//#define HALFSTEPS
 #define FULLSTEPS

/*! \Brief Define IO port and pins
 *
 * Set the desired drive port and pins to support your device
 *
 */
#define SM_PORT PORTC
#define SM_DRIVE DDRC

void sm_driver_Init_IO(void);

//! Position of stepper motor.
extern int stepPosition;

#endif
