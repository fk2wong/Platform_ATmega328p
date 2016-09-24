/*
 * PlatformADC.c
 *
 * Created: 2016-09-11 11:08:26 PM
 *  Author: Felix
 */ 

#include "PlatformADC.h"
#include "PlatformClock.h"
#include "PlatformPowerSave.h"
#include "require_macros.h"
#include "FQuadLogging.h"
#include <avr/io.h>

//======================//
//       Defines        //
//======================//

#define PLATFORM_ADC_INPUT_CLOCK_MIN_HZ  ( 50000  )
#define PLATFORM_ADC_INPUT_CLOCK_MAX_HZ  ( 200000 )

#define PLATFORM_ADC_MAX_PRESCALER_VALUE ( 7 )
#define PLATFORM_ADC_INVALID_PRESCALER   ( PLATFORM_ADC_MAX_PRESCALER_VALUE + 1 )

#define PLATFORM_ADC_INVALID_DIV_FACTOR  ( 0 )

#define PLATFORM_ADC_GET_MASK( X )       ( 1 << ( X ))

#define PLATFORM_ADC_MUX_PIN_MASK        (( 1 << MUX0 ) | ( 1 << MUX1 ) | ( 1 << MUX2 ) | ( 1 << MUX3 ))

#define PLATFORM_ADC_MUX_REF_MASK        (( 1 << REFS0 ) | ( 1 << REFS1 ))
#define PLATFORM_ADC_VCC_AS_AREF         ( 1 << REFS0 )

#define PLATFORM_ADC_DISABLE_ALL_INPUT_BUFFERS (( 1 << ADC0D ) | ( 1 << ADC1D ) | ( 1 << ADC2D ) | ( 1 << ADC3D ) | ( 1 << ADC4D ) | ( 1 << ADC5D ))

//======================//
//   Static Variables   //
//======================//

static uint8_t mPlatformADCInitializedADCs;

//==================================//
//   Static Function Declarations   //
//==================================//

static uint8_t _PlatformADC_GetInputClockPrescaler( void );
static uint8_t _PlatformADC_GetDivisionFactorFromPrescaler( uint8_t inPrescaler );

//==================================//
//   Public Function Definitions    //
//==================================//

PlatformStatus PlatformADC_DisableAllInputBuffers( void )
{
	PlatformStatus status = PlatformStatus_Failed;
	
	// This should only be called if no ADCs are initialized
	require_quiet( mPlatformADCInitializedADCs == 0, exit );
	
	// Disable all input buffers
	DIDR0 = PLATFORM_ADC_DISABLE_ALL_INPUT_BUFFERS;
	
	status = PlatformStatus_Success;
exit:
	return status;
}

PlatformStatus PlatformADC_Init( PlatformADC_t inADC )
{
	PlatformStatus status = PlatformStatus_Failed;
	
	// Check this is a valid ADC
	require_quiet( inADC < PlatformADC_Max, exit );
	
	// Check that this ADC is not already initialized
	require_quiet(( mPlatformADCInitializedADCs & PLATFORM_ADC_GET_MASK( inADC )) == 0, exit );
	
	// If this is the first time any ADC has been initialized:
	if ( mPlatformADCInitializedADCs == 0 )
	{
		// Disable PowerSave
		status = PlatformPowerSave_PowerOnPeripheral( PlatformPowerSavePeripheral_ADC );
		require_noerr_quiet( status, exit );
		
		// Enable the ADC block and configure the ADC clock prescaler
		uint8_t chosenPrescaler = _PlatformADC_GetInputClockPrescaler();
		require_quiet( chosenPrescaler != PLATFORM_ADC_INVALID_PRESCALER, exit );	
		
		ADCSRA |= ( 1 << ADEN ) | chosenPrescaler;
		
		// Set the voltage reference source as Vcc
		ADMUX &= ~PLATFORM_ADC_MUX_REF_MASK;
		ADMUX |= PLATFORM_ADC_VCC_AS_AREF;
	}
	
	// Regardless, enable the specific input buffer
	DIDR0 &= ~PLATFORM_ADC_GET_MASK( inADC );
	
	// Add to the list of initialized ADC inputs
	mPlatformADCInitializedADCs |= PLATFORM_ADC_GET_MASK( inADC );
	
	status = PlatformStatus_Success;
exit:
	return status;
}

PlatformStatus PlatformADC_Read( PlatformADC_t inADC, uint16_t *const outADCValue )
{
	PlatformStatus status = PlatformStatus_Failed;
	
	require_quiet( inADC < PlatformADC_Max, exit );
	require_quiet( outADCValue,             exit );
	
	// Check that this ADC is initialized
	require_quiet( mPlatformADCInitializedADCs & PLATFORM_ADC_GET_MASK( inADC ), exit );
	
	// Sanity check that there is no conversion currently in progress
	require_quiet(( ADCSRA & ( 1 << ADSC )) == 0, exit );
	
	// Clear the previous ADC input MUX selection
	ADMUX &= ~( PLATFORM_ADC_MUX_PIN_MASK );
	
	// Select the ADC input from the MUX
	ADMUX |= inADC;
	
	// Start a new conversion
	ADCSRA |= ( 1 << ADSC );
	
	// Wait for conversion complete
	while( ADCSRA & ( 1 << ADSC ));
	
	// Get the ADC Value; LSB Register must be read first.
	*outADCValue = ADCL;
	*outADCValue |= ADCH << 8;
	
	status = PlatformStatus_Success;
exit:
	return status;
}

PlatformStatus PlatformADC_Deinit( PlatformADC_t inADC )
{
	PlatformStatus status = PlatformStatus_Failed;
	
	// Check this is a valid ADC
	require_quiet( inADC < PlatformADC_Max, exit );
	
	// Check that this ADC is initialized
	require_quiet( mPlatformADCInitializedADCs & PLATFORM_ADC_GET_MASK( inADC ), exit );
	
	// Disable the specific input buffer
	DIDR0 |= PLATFORM_ADC_GET_MASK( inADC );
		
	// Remove from the list of initialized ADC inputs
	mPlatformADCInitializedADCs &= ~PLATFORM_ADC_GET_MASK( inADC );
	
	// If there are no more initialized ADC inputs:
	if ( mPlatformADCInitializedADCs == 0 )
	{
		// Disable the ADC block
		ADCSRA &= ~( 1 << ADEN );
				
		// Enable PowerSave
		status = PlatformPowerSave_PowerOffPeripheral( PlatformPowerSavePeripheral_ADC );
		require_noerr_quiet( status, exit );
	}
	
	status = PlatformStatus_Success;
exit:
	return status;
}

//==================================//
//   Static Function Definitions    //
//==================================//

static uint8_t _PlatformADC_GetInputClockPrescaler( void )
{
	uint8_t minDivisionFactor;
	uint8_t chosenDivisionFactor;
	uint8_t chosenPrescaler = PLATFORM_ADC_INVALID_PRESCALER;
	
	// Calculate the min division factor needed for the prescaler to the ADC input clock.
	minDivisionFactor = F_CPU / PLATFORM_ADC_INPUT_CLOCK_MAX_HZ;
			
	// Loop and find the fastest acceptable prescaler setting
	for ( uint8_t i = 1; i <= PLATFORM_ADC_MAX_PRESCALER_VALUE; i++ )
	{
		chosenDivisionFactor = _PlatformADC_GetDivisionFactorFromPrescaler( i );
		require_quiet( chosenDivisionFactor != PLATFORM_ADC_INVALID_DIV_FACTOR, exit );
		
		if ( chosenDivisionFactor > minDivisionFactor )
		{
			chosenPrescaler = i;
			break;
		}
	}
	
exit:
	return chosenPrescaler;
}

static uint8_t _PlatformADC_GetDivisionFactorFromPrescaler( uint8_t inPrescaler )
{
	uint8_t divFactor = PLATFORM_ADC_INVALID_DIV_FACTOR;
	
	if ( inPrescaler == 0 )
	{
		divFactor = 2;
	}
	else if ( inPrescaler <= PLATFORM_ADC_MAX_PRESCALER_VALUE )
	{
		divFactor = ( 1 << inPrescaler);
	}
	return divFactor;
}