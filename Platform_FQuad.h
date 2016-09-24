/*
 * Platform_FQuad.h
 *
 * Created: 2016-09-06 4:29:33 PM
 *  Author: Felix
 */ 


#ifndef PLATFORM_FQUAD_H_
#define PLATFORM_FQUAD_H_

#include "PlatformInterrupt.h"
#include "PlatformPowerSave.h"
#include "PlatformClock.h"
#include "PlatformGPIO.h"
#include "PlatformRingBuffer.h"
#include "PlatformUART.h"
#include "PlatformADC.h"


typedef enum 
{	
	FQuadGPIO_PadDUp         = PlatformGPIO_PTD2,
	FQuadGPIO_PadDDown       = PlatformGPIO_PTD3,
	FQuadGPIO_PadDLeft       = PlatformGPIO_PTD4,
	FQuadGPIO_PadDRight      = PlatformGPIO_PTD5,
	FQuadGPIO_PadA           = PlatformGPIO_PTD6,
	FQuadGPIO_PadB           = PlatformGPIO_PTD7,
	FQuadGPIO_PadX           = PlatformGPIO_PTC0,
	FQuadGPIO_PadY           = PlatformGPIO_PTC1,
	FQuadGPIO_PadStart       = PlatformGPIO_PTB0,
	FQuadGPIO_PadSelect      = PlatformGPIO_PTB1,
	FQuadGPIO_PadCenter      = PlatformGPIO_PTB2,
	FQuadGPIO_PadLeftBumper  = PlatformGPIO_PTB3,
	FQuadGPIO_PadRightBumper = PlatformGPIO_PTB4,
	FQuadGPIO_TestLED        = PlatformGPIO_PTB5,
} FQuadGPIO_t;

typedef enum 
{
	FQuadADC_PadLeftVertical    = PlatformADC_ADC2,
	FQuadADC_PadLeftHorizontal  = PlatformADC_ADC3,
	FQuadADC_PadRightVertical   = PlatformADC_ADC4,
	FQuadADC_PadRightHorizontal = PlatformADC_ADC5,
} FQuadADC_t;


#endif /* PLATFORM_FQUAD_H_ */