/*
 * PlatformTimer.h
 *
 * This timer runs off the 16-bit Timer/Counter1.
 * This allows users to determine elapsed time between calls to _GetTime().
 * Timer interrupts may be added in the future when their functionality is necessary.
 *
 * Created: 2017-01-18 11:13:55 PM
 *  Author: Felix
 */ 


#ifndef PLATFORMTIMER_H_
#define PLATFORMTIMER_H_

#include "PlatformStatus.h"
#include <stdint.h>

PlatformStatus PlatformTimer_Init( void );

PlatformStatus PlatformTimer_GetTime( uint32_t * const outTime );

PlatformStatus PlatformTimer_Reset( void );

PlatformStatus PlatformTimer_Deinit( void );


#endif /* PLATFORMTIMER_H_ */