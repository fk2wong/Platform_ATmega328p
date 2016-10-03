/*
 * PlatformI2C.c
 *
 * Created: 2016-09-26 11:39:35 PM
 *  Author: Felix
 */ 

#include "PlatformI2C.h"
#include "PlatformStatus.h"
#include "PlatformPowerSave.h"
#include "PlatformClock.h"
#include "require_macros.h"
#include <avr/io.h>
#include <stdbool.h>

#define PLATFORM_I2C_FAST_MODE_HZ ( 400000 )
#define PLATFORM_I2C_WRITE_BIT    ( 0 )
#define PLATFORM_I2C_READ_BIT     ( 1 )

#define PLATFORM_I2C_TWBR_MAX     ( 0xFF )
#define PLATFORM_I2C_TWBR_MIN     ( 0x00 )

#define GET_TWSR_STATUS_CODE() ( TWSR & (( 1 << TWS7 ) | ( 1 << TWS6 ) | ( 1 << TWS5 ) | ( 1 << TWS4 ) | ( 1 << TWS3 )))

enum TWSRStatus
{
	TWSRStatus_StartConditionTransmitted         = 0x08,
	TWSRStatus_RepeatedStartConditionTransmitted = 0x10,
	TWSRStatus_SLAW_ACKReceived                  = 0x18,
	TWSRStatus_SLAW_NACKReceived                 = 0x20,
	TWSRStatus_DataACKReceived                   = 0x28,
	TWSRStatus_DataNACKReceived                  = 0x30,
	TWSRStatus_ArbitrationLost                   = 0x38,
	TWSRStatus_SLAR_ACKReceived                  = 0x40,
	TWSRStatus_SLAR_NACKReceived                 = 0x48,
	TWSRStatus_DataReceivedAndACKSent            = 0x50,
	TWSRStatus_DataReceivedAndNACKSent           = 0x58,
		
};

static const uint8_t kPlatformI2CBitRatePrescalers[] = { 1, 4, 16, 64 };
static bool mPlatformI2CIsInitialized;

static PlatformStatus        _PlatformI2C_ReadByte( uint8_t *const outDataByte, const bool isLastByteToRead );
static PlatformStatus        _PlatformI2C_WriteByte( const uint8_t inDataByte );
static inline PlatformStatus _PlatformI2C_SendRegisterAddress( const uint8_t inRegisterAddress );
static PlatformStatus        _PlatformI2C_SendSlaveAddressAndReadWriteBit( const uint8_t inDeviceAddr, const bool inReadWriteBit );
static void                  _PlatformI2C_SendStopCondition( void );
static PlatformStatus        _PlatformI2C_SendStartCondition( const bool isRepeatedStart );
static PlatformStatus        _PlatformI2C_GetClockPrescalerBitsAndBitRateValues( uint32_t inCPUFreq, uint8_t *const outPrescalerBits, uint8_t *const outBitRateVal );

PlatformStatus PlatformI2C_Init( void )
{
	PlatformStatus status = PlatformStatus_Failed;
	
	uint8_t clockPrescalerBits;
	uint8_t bitRateRegisterValue;
	
	require_action_quiet( !mPlatformI2CIsInitialized, exit, status = PlatformStatus_AlreadyInitialized );
	
	// Enable Power to the I2C peripheral
	status = PlatformPowerSave_PowerOnPeripheral( PlatformPowerSavePeripheral_I2C );
	require_noerr_quiet( status, exit );
	
	// Get the prescaler and bit rate values that create an SCL frequency of 400kHz
	status = _PlatformI2C_GetClockPrescalerBitsAndBitRateValues( F_CPU, &clockPrescalerBits, &bitRateRegisterValue );
	require_noerr_quiet( status, exit );
	
	// Set the prescaler bits
	TWSR |= clockPrescalerBits;
	
	// Set the bit rate register
	TWBR = bitRateRegisterValue;
	
	// Enable the I2C peripheral
	TWCR = ( 1 << TWEN );
	
	mPlatformI2CIsInitialized = true;
	
	status = PlatformStatus_Success;
exit:
	return status;
}

PlatformStatus PlatformI2C_Deinit( void )
{
	PlatformStatus status = PlatformStatus_Failed;
	
	require_action_quiet( mPlatformI2CIsInitialized, exit, status = PlatformStatus_NotInitialized );
	
	// Disable the I2C peripheral
	TWCR &= ~( 1 << TWEN );
	
	// Disable power to the peripheral
	status = PlatformPowerSave_PowerOffPeripheral( PlatformPowerSavePeripheral_I2C );
	require_noerr_quiet( status, exit );
	
	mPlatformI2CIsInitialized = false;
	
	status = PlatformStatus_Success;
exit:
	return status;
}

PlatformStatus PlatformI2C_WriteByte( const uint8_t inDeviceAddr, const uint8_t inRegisterAddress, const uint8_t inDataByte )
{
	return PlatformI2C_Write( inDeviceAddr, inRegisterAddress, &inDataByte, 1 );
}

PlatformStatus PlatformI2C_Write( const uint8_t inDeviceAddr, const uint8_t inRegisterAddress, const uint8_t *const inData, const size_t inDataLen )
{
	PlatformStatus status = PlatformStatus_Failed;
	bool startConditionSent = false;
	
	require_quiet( inData,    exit );
	require_quiet( inDataLen, exit );
	
	require_action_quiet( mPlatformI2CIsInitialized, exit, status = PlatformStatus_NotInitialized );
	
	// Sanity check that there is no current I2C action in progress, and that the write collision bit is cleared
	require_quiet( !( TWCR & ( 1 << TWINT )), exit );
	require_quiet( !( TWCR & ( 1 << TWWC )),  exit );
	
	status = _PlatformI2C_SendStartCondition( false );
	require_noerr_quiet( status, exit );
	startConditionSent = true;
	
	status = _PlatformI2C_SendSlaveAddressAndReadWriteBit( inDeviceAddr, PLATFORM_I2C_WRITE_BIT );
	require_noerr_quiet( status, exit );
	
	status = _PlatformI2C_SendRegisterAddress( inRegisterAddress );
	require_noerr_quiet( status, exit );
	
	// Send each byte of data
	for ( size_t i = 0; i < inDataLen; i++ )
	{	
		_PlatformI2C_WriteByte( inData[i] );
		require_noerr_quiet( status, exit );
	}
	
	status = PlatformStatus_Success;
exit:
	if ( startConditionSent )
	{
		_PlatformI2C_SendStopCondition();
	}
	
	return status;
}

PlatformStatus PlatformI2C_Read( const uint8_t inDeviceAddr, const uint8_t inRegisterAddress, uint8_t *const outData, const size_t inDataLen )
{
	PlatformStatus status = PlatformStatus_Failed;
	bool startConditionSent = false;
		
	require_quiet( outData,   exit );
	require_quiet( inDataLen, exit );
	
	require_action_quiet( mPlatformI2CIsInitialized, exit, status = PlatformStatus_NotInitialized );
		
	// Sanity check that there is no current I2C action in progress, and that the write collision bit is cleared
	require_quiet( !( TWCR & ( 1 << TWINT )), exit );
	require_quiet( !( TWCR & ( 1 << TWWC )),  exit );
	
	status = _PlatformI2C_SendStartCondition( false );
	require_noerr_quiet( status, exit );
	startConditionSent = true;
	
	status = _PlatformI2C_SendSlaveAddressAndReadWriteBit( inDeviceAddr, PLATFORM_I2C_WRITE_BIT );
	require_noerr_quiet( status, exit );
	
	status = _PlatformI2C_SendRegisterAddress( inRegisterAddress );
	require_noerr_quiet( status, exit );
	
	// Send Repeated Start Condition 
	status = _PlatformI2C_SendStartCondition( true );
	require_noerr_quiet( status, exit );
	
	status = _PlatformI2C_SendSlaveAddressAndReadWriteBit( inDeviceAddr, PLATFORM_I2C_READ_BIT );
	require_noerr_quiet( status, exit );
	
	// Read each byte
	for ( size_t i = 0; i < inDataLen; i++ )
	{
		bool isLastByte = ( i == ( inDataLen - 1 )) ? true : false;
		
		status = _PlatformI2C_ReadByte( &outData[i], isLastByte );
		require_noerr_quiet( status, exit );
	}
	
	status = PlatformStatus_Success;
	
exit:
	if ( startConditionSent )
	{
		_PlatformI2C_SendStopCondition();
	}
	return status;
}

static PlatformStatus _PlatformI2C_SendStartCondition( const bool isRepeatedStart )
{
	PlatformStatus status = PlatformStatus_Failed;
		
	// Clear any stop condition and send start condition // TODO does this need to be on different clock cycles?
	TWCR = ( TWCR & ~( 1 << TWSTO )) | ( 1 << TWSTA ) | ( 1 << TWINT );
		
	// Wait for interrupt flag, indicating the start condition was sent
	while ( !( TWCR & ( 1 << TWINT )));
		
	// Check the status of the start condition
	if ( isRepeatedStart )
	{
		require_quiet( GET_TWSR_STATUS_CODE() == TWSRStatus_RepeatedStartConditionTransmitted, exit );	
	}
	else
	{
		require_quiet( GET_TWSR_STATUS_CODE() == TWSRStatus_StartConditionTransmitted, exit );
	}
	
	status = PlatformStatus_Success;
exit:
	return status;
}

static void _PlatformI2C_SendStopCondition( void )
{
	// Clear start condition, disable ACK, write stop condition, set interrupt flag to send stop condition
	TWCR = ( TWCR & ~(( 1 << TWSTA ) | ( 1 << TWEA ))) | ( 1 << TWSTO ) | ( 1 << TWINT );
}

static PlatformStatus _PlatformI2C_SendSlaveAddressAndReadWriteBit( const uint8_t inDeviceAddr, const bool inReadWriteBit )
{
	PlatformStatus status = PlatformStatus_Failed;
	
	// Set the device address and write/read bit (SLA+R/W)
	TWDR = ( inDeviceAddr << 1 ) | inReadWriteBit;
	
	// Clear any Start/Stop Condition and write the interrupt flag in order to send SLA+W
	TWCR = ( TWCR & ~(( 1 << TWSTA ) | ( 1 << TWSTO ))) | ( 1 << TWINT );
	
	// Wait for interrupt flag, indicating SLA+W was sent
	while ( !( TWCR & ( 1 << TWINT )));
	
	// Check if ACK was received
	if ( inReadWriteBit == PLATFORM_I2C_WRITE_BIT )
	{
		require_quiet( GET_TWSR_STATUS_CODE() == TWSRStatus_SLAW_ACKReceived, exit );
	}
	else 
	{
		require_quiet( GET_TWSR_STATUS_CODE() == TWSRStatus_SLAR_ACKReceived, exit );
	}
	
	status = PlatformStatus_Success;
exit:
	return status;
}

static inline PlatformStatus _PlatformI2C_SendRegisterAddress( const uint8_t inRegisterAddress )
{
	return _PlatformI2C_WriteByte( inRegisterAddress );
}

static PlatformStatus _PlatformI2C_WriteByte( const uint8_t inDataByte )
{
	PlatformStatus status = PlatformStatus_Failed;
	
	// Set the data to be sent
	TWDR = inDataByte;
	
	// Clear any start/stop condition and write the interrupt flag in order to send the data
	TWCR = ( TWCR & ~(( 1 << TWSTA ) | ( 1 << TWSTO ))) | ( 1 << TWINT );
	
	// Wait for interrupt flag, indicating the data was sent
	while ( !( TWCR & ( 1 << TWINT )));
	
	// Check if ACK was received
	require_quiet( GET_TWSR_STATUS_CODE() == TWSRStatus_DataACKReceived, exit );
	
	status = PlatformStatus_Success;
exit:
	return status;
}

static PlatformStatus _PlatformI2C_ReadByte( uint8_t *const outDataByte, const bool isLastByteToRead )
{
	PlatformStatus status = PlatformStatus_Failed;
		
	require_quiet( outDataByte, exit );
	
	// If this is the last byte to read, set up the TWCR to send a NACK; otherwise send an ACK
	if ( isLastByteToRead )
	{
		TWCR &= ~( 1 << TWEA );
	}
	else
	{
		TWCR |= ( 1 << TWEA );	
	}
		
	// Clear any start/stop condition and write the interrupt flag in order to receive data
	TWCR = ( TWCR & ~(( 1 << TWSTA ) | ( 1 << TWSTO ))) | ( 1 << TWINT );
		
	// Wait for interrupt flag, indicating the data was sent
	while ( !( TWCR & ( 1 << TWINT )));
		
	// Check if data was received successfully
	if ( isLastByteToRead )
	{
		require_quiet( GET_TWSR_STATUS_CODE() == TWSRStatus_DataReceivedAndNACKSent, exit );
	}
	else
	{
		require_quiet( GET_TWSR_STATUS_CODE() == TWSRStatus_DataReceivedAndACKSent, exit );
	}
	
	// Get read data
	*outDataByte = TWDR;
		
	status = PlatformStatus_Success;
exit:
	return status;
}

static PlatformStatus _PlatformI2C_GetClockPrescalerBitsAndBitRateValues( uint32_t inCPUFreq, uint8_t *const outPrescalerBits, uint8_t *const outBitRateVal )
{
	PlatformStatus status = PlatformStatus_Failed;
	
	uint32_t currentPrescalerUpperSpeed;
	uint32_t currentPrescalerLowerSpeed;
	uint8_t  prescalerIndex = 0;
	
	bool prescalerFound = false;
	
	require_quiet( outPrescalerBits,  exit );
	require_quiet( outBitRateVal, exit );
	
	for ( uint8_t prescalerIndex = 0; prescalerIndex < sizeof( kPlatformI2CBitRatePrescalers ); prescalerIndex++ )
	{
		// The below formula is taken from Section 21.5.2 of the ATmega328p datasheet
		currentPrescalerUpperSpeed = inCPUFreq / ( 16 + ( 2 * PLATFORM_I2C_TWBR_MIN * kPlatformI2CBitRatePrescalers[ prescalerIndex ] ));
		currentPrescalerLowerSpeed = inCPUFreq / ( 16 + ( 2 * PLATFORM_I2C_TWBR_MAX * kPlatformI2CBitRatePrescalers[ prescalerIndex ] ));
		
		// If the current prescaler can accomodate the 400kHz of fast mode, break and use it
		if (( currentPrescalerUpperSpeed >= PLATFORM_I2C_FAST_MODE_HZ ) || ( PLATFORM_I2C_FAST_MODE_HZ >= currentPrescalerLowerSpeed ))
		{
			prescalerFound = true;
			break;
		}
	}
	
	require_quiet( prescalerFound, exit );
	
	// Set the returned prescaler bits as the index 0-3 into the array kPlatformI2CBitRatePrescalers
	*outPrescalerBits = prescalerIndex;
	
	// The below formula is rearranged from Section 21.5.2 from the Atmega328p datasheet
	*outBitRateVal    = (( inCPUFreq / PLATFORM_I2C_FAST_MODE_HZ ) - 16 ) / ( 2 * kPlatformI2CBitRatePrescalers[ prescalerIndex ] );
	
	status = PlatformStatus_Success;
exit:
	return status;
}