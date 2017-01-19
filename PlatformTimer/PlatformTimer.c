/*
 * PlatformTimer.c
 *
 * Created: 2017-01-18 11:13:42 PM
 *  Author: Felix
 */ 

#include "PlatformTimer.h"
#include "PlatformClock.h"
#include "PlatformPowerSave.h"
#include "PlatformInterrupt.h"
#include "require_macros.h"
#include <stdbool.h>

#define PLATFORM_TIMER_MAX_VAL ( 0xFFFF )

static bool mPlatformTimerInitialized;
static bool mPlatformTimerEnabledGlobalInterrupts;
static uint64_t mPlatformTimerCurrentMilliseconds;

PlatformStatus PlatformTimer_Init( void )
{
	PlatformStatus status = PlatformStatus_Failed;
	uint16_t topVal;
	
	// Check if already initialized
	require_action_quiet( !mPlatformTimerInitialized, exit, status = PlatformStatus_AlreadyInitialized );
	
	// Power on this peripheral
	status = PlatformPowerSave_PowerOnPeripheral( PlatformPowerSavePeripheral_Timer1 );
	require_noerr_quiet( status, exit );
	
	// Set CTC mode
	TCCR1A &= ~(( 1 << WGM11 ) | ( 1 << WGM10 ));
	TCCR1B &= ~( 1 << WGM13 );
	TCCR1B |=  ( 1 << WGM12 );
	
	// Set clock source, no prescaling
	TCCR1B &= ~(( 1 << CS12 ) | ( 1 << CS11 ));
	TCCR1B |= ( 1 << CS10 );
	
	// Set the TOP value such that the timer overflows every 1ms
	topVal = ( uint16_t )( F_CPU / 1000 );

	OCR1AH = ( topVal >> 8 ) & 0xFF;
	OCR1AL = topVal & 0xFF;
	
	// Enable global interrupts, if not already
	if ( !PlatformInterrupt_AreGlobalInterruptsEnabled() )
	{
		mPlatformTimerEnabledGlobalInterrupts = true;
		PlatformInterrupt_EnableGlobalInterrupts();
	}
	
	// Enable interrupt on timer TOP value
	TIMSK1 |= 1 << OCIE1A;
	
	mPlatformTimerInitialized = true;
	status = PlatformStatus_Success;
exit:
	return status; 
}

PlatformStatus PlatformTimer_GetTime( uint64_t * const outTime )
{
	PlatformStatus status = PlatformStatus_NotInitialized;
	
	// Check if initialized
	require_quiet( mPlatformTimerInitialized, exit );
	require_quiet( outTime, exit );
	
	// Read millisecond count
	PlatformInterrupt_DisableGlobalInterrupts();
	*outTime = mPlatformTimerCurrentMilliseconds;
	PlatformInterrupt_EnableGlobalInterrupts();
	
	status = PlatformStatus_Success;
exit:
	return status;
}

PlatformStatus PlatformTimer_Reset( void )
{
	PlatformStatus status = PlatformStatus_NotInitialized;
		
	// Check if initialized
	require_quiet( mPlatformTimerInitialized, exit );
		
	// Reset millisecond count
	PlatformInterrupt_DisableGlobalInterrupts();
	mPlatformTimerCurrentMilliseconds = 0;
	PlatformInterrupt_EnableGlobalInterrupts();
		
	status = PlatformStatus_Success;
exit:
	return status;
}

PlatformStatus PlatformTimer_Deinit( void )
{
	PlatformStatus status = PlatformStatus_Failed;
	
	require_action_quiet( mPlatformTimerInitialized, exit, status = PlatformStatus_NotInitialized );
	
	// Disable interrupts, if we enabled them during initialization
	if ( mPlatformTimerEnabledGlobalInterrupts )
	{
		PlatformInterrupt_DisableGlobalInterrupts();
	}
	
	// Turn off timer clock
	TCCR1B &= ~(( 1 << CS12 ) | ( 1 << CS11 ) | ( 1 << CS10 ));
	
	// Disable this peripheral
	status = PlatformPowerSave_PowerOffPeripheral( PlatformPowerSavePeripheral_Timer1 );
	require_noerr_quiet( status, exit );
	
	mPlatformTimerInitialized = false;
	status = PlatformStatus_Success;
exit:
	return status;
}

ISR( TIMER1_COMPA_vect )
{
	// Update the millisecond count
	PlatformInterrupt_DisableGlobalInterrupts();
	mPlatformTimerCurrentMilliseconds++;
	PlatformInterrupt_EnableGlobalInterrupts();
}