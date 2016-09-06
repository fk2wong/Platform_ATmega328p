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

void PlatformGPIO_InitAllGPIOs( void );

void PlatformGPIO_Configure( PlatformGPIO_t inGPIO, PlatformGPIOConfig_t inConfig );

void PlatformGPIO_Toggle( PlatformGPIO_t inGPIO );

#endif /* PLATFORMGPIO_H_ */