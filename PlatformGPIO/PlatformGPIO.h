/*
 * PlatformGPIO.h
 *
 * Created: 2016-09-06 12:20:20 PM
 *  Author: Felix
 */ 


#ifndef PLATFORMGPIO_H_
#define PLATFORMGPIO_H_

#include "Platform.h"
#include <stdbool.h>

typedef enum
{
	PlatformGPIO_PTD2,
	PlatformGPIO_PTD3,
	PlatformGPIO_PTD4,
	PlatformGPIO_PTD5,
	PlatformGPIO_PTD6,
	PlatformGPIO_PTD7,
	PlatformGPIO_PTC0,
	PlatformGPIO_PTC1,
	PlatformGPIO_PTB0,
	PlatformGPIO_PTB1,
	PlatformGPIO_PTB2,
	PlatformGPIO_PTB3,
	PlatformGPIO_PTB4,
	PlatformGPIO_PTB5,	
} PlatformGPIO_t;

typedef enum
{
	PlatformGPIOConfig_InputHighZ,
	PlatformGPIOConfig_InputPullUp,	
	PlatformGPIOConfig_Output,
} PlatformGPIOConfig_t;

/*!
 *\brief    Inits all GPIOs to their default setting, specified in PlatformGPIO.c
 */
void PlatformGPIO_InitAllGPIOs( void );

/*!
 *\brief    Configures a GPIO to a setting. 
 *
 *\param    inGPIO   - GPIO pin to configure.
 *\param    inConfig - New configuration for the GPIO.
 *
 */
void PlatformGPIO_Configure( PlatformGPIO_t inGPIO, PlatformGPIOConfig_t inConfig );

/*!
 *\brief    Outputs logic high on a GPIO. Ensure that the GPIO is configured to be an output prior to this call.
 *
 *\param    inGPIO - GPIO pin to output HIGH.
 */
void PlatformGPIO_OutputHigh( PlatformGPIO_t inGPIO );

/*!
 *\brief    Outputs logic low on a GPIO. Ensure that the GPIO is configured to be an output prior to this call.
 *
 *\param    inGPIO - GPIO pin to output LOW.
 */
void PlatformGPIO_OutputLow( PlatformGPIO_t inGPIO );

/*!
 *\brief    Toggles the output on a GPIO. Ensure that the GPIO is configured to be an output prior to this call.
 *
 *\param    inGPIO - GPIO pin to toggle.
 */
void PlatformGPIO_Toggle( PlatformGPIO_t inGPIO );

/*!
 *\brief    Read the logic level on a GPIO configured as input.
 *
 *\param    inGPIO - GPIO pin to read.
 */
bool PlatformGPIO_GetInput( PlatformGPIO_t inGPIO );

#endif /* PLATFORMGPIO_H_ */