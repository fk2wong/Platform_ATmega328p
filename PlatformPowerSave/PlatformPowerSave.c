/*
 * PlatformPowerSave.c
 *
 * Created: 2016-09-10 7:00:39 PM
 *  Author: Felix
 */ 

#include "PlatformPowerSave.h"
#include "require_macros.h"
#include <avr/io.h>

PlatformStatus PlatformPowerSave_PowerOnAllPeripherals( void )
{
	PlatformStatus status = PlatformStatus_Failed;
	
	for ( uint8_t i = 0; i < PlatformPowerSavePeripheral_MaxPeripherals; i++ )
	{
		status = PlatformPowerSave_PowerOnPeripheral( i );
		require_noerr_quiet( status, exit );
	}
	
exit:
	return status;
}

PlatformStatus PlatformPowerSave_PowerOffAllPeripherals( void )
{
	PlatformStatus status = PlatformStatus_Failed;
	
	for ( uint8_t i = 0; i < PlatformPowerSavePeripheral_MaxPeripherals; i++ )
	{
		status = PlatformPowerSave_PowerOffPeripheral( i );
		require_noerr_quiet( status, exit );
	}

exit:
	return status;
}

PlatformStatus PlatformPowerSave_PowerOnPeripheral( PlatformPowerSavePeripheral_t inPeripheral )
{
	PlatformStatus status = PlatformStatus_Failed;
	
	switch ( inPeripheral )
	{
		case PlatformPowerSavePeripheral_ADC:
		{
			PRR &= ~( 1 << PRADC );
			break;
		}
		case PlatformPowerSavePeripheral_USART:
		{
			PRR &= ~( 1 << PRUSART0 );
			break;
		}
		case PlatformPowerSavePeripheral_SPI:
		{
			PRR &= ~( 1 << PRSPI );
			break;
		}
		case PlatformPowerSavePeripheral_I2C:
		{
			PRR &= ~( 1 << PRTWI );
			break;
		}
		case PlatformPowerSavePeripheral_Timer0:
		{
			PRR &= ~( 1 << PRTIM0 );
			break;
		}
		case PlatformPowerSavePeripheral_Timer1:
		{
			PRR &= ~( 1 << PRTIM1 );
			break;
		}
		case PlatformPowerSavePeripheral_Timer2:
		{
			PRR &= ~( 1 << PRTIM2 );
			break;
		}
		default :
		{
			goto exit;
		}
	}
	
	status = PlatformStatus_Success;
exit:
	return status;
}

PlatformStatus PlatformPowerSave_PowerOffPeripheral( PlatformPowerSavePeripheral_t inPeripheral )
{
	PlatformStatus status = PlatformStatus_Failed;
	
	switch ( inPeripheral )
	{
		case PlatformPowerSavePeripheral_ADC:
		{
			PRR |= 1 << PRADC;
			break;
		}
		case PlatformPowerSavePeripheral_USART:
		{
			PRR |= 1 << PRUSART0;
			break;
		}
		case PlatformPowerSavePeripheral_SPI:
		{
			PRR |= 1 << PRSPI;
			break;
		}
		case PlatformPowerSavePeripheral_I2C:
		{
			PRR |= 1 << PRTWI;
			break;
		}
		case PlatformPowerSavePeripheral_Timer0:
		{
			PRR |= 1 << PRTIM0;
			break;
		}
		case PlatformPowerSavePeripheral_Timer1:
		{
			PRR |= 1 << PRTIM1;
			break;
		}
		case PlatformPowerSavePeripheral_Timer2:
		{
			PRR |= 1 << PRTIM2;
			break;
		}
		default :
		{
			goto exit;
		}
	}
	
	status = PlatformStatus_Success;
exit:
	return status;
}