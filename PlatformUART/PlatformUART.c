/*
 * PlatformUART.c
 *
 * Created: 2016-09-07 5:27:35 PM
 *  Author: Felix
 */ 


#include "PlatformUART.h"
#include "PlatformClock.h"
#include "PlatformPowerSave.h"
#include "PlatformInterrupt.h"
#include "require_macros.h"
#include <stdbool.h>

//===============//
//    Defines    //
//===============//

#ifndef F_CPU
#define F_CPU 16000000
#error F_CPU Not defined! Please include PlatformClock.h.
#endif

#define PLATFORM_UART_UBRR0L_MASK       ( 0xFF )
#define PLATFORM_UART_UBRR0H_MASK       ( 0x0F )
#define PLATFORM_UART_BAUD_RATE_REG_MAX (( PLATFORM_UART_UBRR0H_MASK << 8 ) | PLATFORM_UART_UBRR0L_MASK )

#define PLATFORM_UART_DATA_REG_EMPTY    ( UCSR0A & ( 1 << UDRE0 ))

#define PLATFORM_UART_RX_FIFO_SIZE      ( 3 )

//==================================//
//    Static Structs & Variables    //
//==================================//

static bool                mUARTIsInitialized;
static PlatformRingBuffer* mRXRingBuffer;

//===================================//
//    Public Function Definitions    //
//===================================//

PlatformStatus PlatformUART_Init( uint32_t inBaudRate, PlatformRingBuffer *const inRingBuffer )
{
	PlatformStatus status = PlatformStatus_Failed;
	uint16_t baudRateRegVal;
	
	require_quiet( !mUARTIsInitialized, exit );
	require_quiet( inRingBuffer,        exit );

	// Disable Power Reduction (enable power) to the UART
	status = PlatformPowerSave_PowerOnPeripheral( PlatformPowerSavePeripheral_USART );
	require_noerr_quiet( status, exit );
	
	// Initialize UCSR0A as non-double transmission speed, single processor mode ( All default )
	
	// Initialize UCSR0B bit UCSZ02 as 0 to set 8-bit width ( Default )
	
	// Initialize UCSR0C as Asynchronous (UART); Parity Disabled; 1 Stop bit; 8-bit width
	UCSR0C = ( 1 << UCSZ00 ) | ( 1 << UCSZ01 );
	
	// Calculate Baud Rate Register Values. Formula reference from ATMega328P datasheet table 24-1, with float to int conversion.
	baudRateRegVal = ( uint16_t )((( float )( F_CPU ) / ( float )( inBaudRate ) / 16.0f ) + 0.5f ) - 1;
	
	// Verify that the baud rate register value is within range
	require_quiet( baudRateRegVal <= PLATFORM_UART_BAUD_RATE_REG_MAX, exit );
	
	// Set the baud rate high and low registers
	UBRR0L = baudRateRegVal & PLATFORM_UART_UBRR0L_MASK;
	UBRR0H = ( baudRateRegVal >> 8 ) & PLATFORM_UART_UBRR0H_MASK;
	
	// Enable the UART transmitter and receiver
	UCSR0B = ( 1 << TXEN0 ) | ( 1 << RXEN0 );
	
	// Save the ring buffer for RX data
	mRXRingBuffer = inRingBuffer;
		
	// Since we are using a ring buffer, allow the RX Complete ISR to read the data into the ring buffer
	UCSR0B |= ( 1 << RXCIE0 );
	PlatformInterrupt_EnableGlobalInterrupts();
	
	mUARTIsInitialized = true;
	
	status = PlatformStatus_Success;
exit:
	return status;
}

PlatformStatus PlatformUART_Transmit( uint8_t* inBuffer, size_t inBufferLen )
{
	PlatformStatus status = PlatformStatus_Failed;
	
	require_quiet( mUARTIsInitialized, exit );
	require_quiet( inBuffer,           exit );
	require_quiet( inBufferLen,        exit );
	
	// Sanity check that the data register is empty. It should never be full at this point.
	require_quiet( PLATFORM_UART_DATA_REG_EMPTY, exit );
	
	// Send each byte in the buffer
	for ( uint8_t i = 0; i < inBufferLen; i++ )
	{	
		// Place next byte into I/O data register
		UDR0 = inBuffer[i];
		
		// Wait for the data register to be empty before sending it the next byte
		while( !PLATFORM_UART_DATA_REG_EMPTY );
	}
	
	status = PlatformStatus_Success;
exit:
	return status;
}

PlatformStatus PlatformUART_Receive( uint8_t* const outBuffer, size_t inRequestedLen )
{
	PlatformStatus status = PlatformStatus_Failed;
	
	require_quiet( mUARTIsInitialized, exit );
	require_quiet( outBuffer,      exit );
	require_quiet( inRequestedLen, exit );
	
	// Read data from the ring buffer
	status = PlatformRingBuffer_ReadBuffer( mRXRingBuffer, outBuffer, inRequestedLen );
	require_noerr( status, exit );
	
	status = PlatformStatus_Success;
exit:
	return status;
}

ISR( USART_RX_vect )
{	
	// Push the RX byte into the ring buffer. 
	// If there is more than one byte in the RX FIFO, this ISR will be called again after it returns.
	PlatformRingBuffer_WriteByte( mRXRingBuffer, UDR0 );
}