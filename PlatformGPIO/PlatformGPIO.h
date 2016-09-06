/*
 * PlatformGPIO.h
 *
 * Created: 2016-09-06 12:20:20 PM
 *  Author: Felix
 */ 


#ifndef PLATFORMGPIO_H_
#define PLATFORMGPIO_H_

#include "Platform.h"

typedef enum
{
	PlatformGPIOConfig_InputHighZ,
	PlatformGPIOConfig_InputPullUp,	
	PlatformGPIOConfig_Output,
} PlatformGPIOConfig_t;

void PlatformGPIO_Configure( PlatformGPIO_t inGPIO, PlatformGPIOConfig_t inConfig );

void PlatformGPIO_Toggle( PlatformGPIO_t inGPIO );

#endif /* PLATFORMGPIO_H_ */