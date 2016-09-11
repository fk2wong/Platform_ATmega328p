/*
 * PlatformRingBuffer.c
 *
 * Created: 2016-09-08 7:57:40 AM
 *  Author: Felix
 */ 

#include "PlatformRingBuffer.h"
#include "PlatformInterrupt.h"
#include "FMemory.h"
#include "require_macros.h"
#include "FUtilities.h"
#include <string.h>
#include <stdbool.h>

//===============//
//    Defines    //
//===============//

// The head points to the next available buffer. If the size of the buffer is 32 (indexes 0-31) and index 31 is written to, 
// then head and tail will be at index 0 after wrapping. This is identical to the start condition, where head and tail are at index 0.
// Thus there needs to be an extra byte (32) which the head will point to after all bytes are written to.
#define PLATFORM_RING_BUFFER_OVERHEAD_BYTES ( 1 )

//===========================//
//    Struct Delcarations    //
//===========================//

struct PlatformRingBufferStruct
{
	size_t   bufferSize;
	uint32_t tailIndex;
	volatile uint32_t headIndex; // Can be changed in ISR
	volatile uint8_t *buffer;    // Contents can be changed in ISR
};

//====================================//
//    Static Function Declarations    //
//====================================//

static inline size_t _PlatformRingBuffer_GetNumFreeBytes( PlatformRingBuffer *const inRingBuffer );
static inline size_t _PlatformRingBuffer_GetNumUsedBytes( PlatformRingBuffer *const inRingBuffer );
static inline void   _PlatformRingBuffer_UpdateHeadIndex( PlatformRingBuffer *const inRingBuffer, size_t inSizeToIncrease );
static inline void   _PlatformRingBuffer_UpdateTailIndex( PlatformRingBuffer *const inRingBuffer, size_t inSizeToIncrease );

//====================================//
//    Public Function Definitions     //
//====================================//

PlatformRingBuffer * PlatformRingBuffer_Create( size_t inBufferSize )
{
	PlatformRingBuffer* newRingBuf    = NULL;
	uint8_t*            newByteBuffer = NULL;
	
	require_quiet( inBufferSize <= PLATFORM_RING_BUFFER_MAX_SIZE, exit );
	
	// Allocate the byte buffer. Needs one extra byte to differentiate between full and empty.
	newByteBuffer = FMemoryAlloc( inBufferSize + PLATFORM_RING_BUFFER_OVERHEAD_BYTES );
	require_quiet( newByteBuffer, exit );
	
	// Allocate new ring buffer struct
	newRingBuf = FMemoryAlloc( sizeof( struct PlatformRingBufferStruct ));
	require_quiet( newRingBuf, exit );
	
	// Initialize internal struct
	newRingBuf->bufferSize = inBufferSize + PLATFORM_RING_BUFFER_OVERHEAD_BYTES;
	newRingBuf->headIndex  = 0;
	newRingBuf->tailIndex  = 0;
	newRingBuf->buffer     = newByteBuffer;
	
exit:
	// If allocation for the struct failed but the byte buffer was still created, free it
	if ( !newRingBuf && newByteBuffer )
	{
		FMemoryFreeAndNULLPtr( &newByteBuffer );
	}
	return newRingBuf;
}


PlatformStatus PlatformRingBuffer_WriteBuffer( PlatformRingBuffer *const inRingBuffer, const uint8_t *const inData, size_t inDataLen )
{
	PlatformStatus status = PlatformStatus_Failed;
	size_t sizeToCopy;
	size_t sizeCopied;
	
	bool didDisableInterrupts = false;
	
	require_quiet( inRingBuffer, exit );
	require_quiet( inData,       exit );
	require_quiet( inDataLen,    exit );
	
	// Check we have enough room in the buffer
	require_quiet( _PlatformRingBuffer_GetNumFreeBytes( inRingBuffer ) >= inDataLen, exit );
	
	// Disable Global Interrupts, if enabled
	if ( PlatformInterrupt_AreGlobalInterruptsEnabled() )
	{
		PlatformInterrupt_DisableGlobalInterrupts();
		didDisableInterrupts = true;
	}
	
	// Copy the data, no further than final byte slot in the ring buffer
	sizeToCopy = MAX( inDataLen, inRingBuffer->bufferSize - inRingBuffer->headIndex + 1 );
	
	memcpy( (void*)&inRingBuffer->buffer[ inRingBuffer->headIndex ], inData, sizeToCopy );
	
	// If there is more data to copy after that, then wrap the buffer around index 0 and copy the rest.
	if ( sizeToCopy < inDataLen )
	{
		sizeCopied = sizeToCopy;
		sizeToCopy = sizeCopied - inDataLen;
		memcpy( (void*)&inRingBuffer->buffer[0], &inData[ sizeCopied ], sizeToCopy );
	}

	// Update the head index
	_PlatformRingBuffer_UpdateHeadIndex( inRingBuffer, inDataLen );
	
	status = PlatformStatus_Success;
	
exit:
	// Enable global interrupts, if we disabled them
	if ( didDisableInterrupts )
	{
		PlatformInterrupt_EnableGlobalInterrupts();
	}
	
	return status;
}

PlatformStatus PlatformRingBuffer_WriteByte( PlatformRingBuffer* const inRingBuffer, const uint8_t inData )
{
	PlatformStatus status = PlatformStatus_Failed;
	bool didDisableInterrupts = false;
		
	require_quiet( inRingBuffer, exit );
		
	// Check we have enough room in the buffer
	require_quiet( _PlatformRingBuffer_GetNumFreeBytes( inRingBuffer ) > 0, exit );
		
	// Disable Global Interrupts, if enabled
	if ( PlatformInterrupt_AreGlobalInterruptsEnabled() )
	{
		PlatformInterrupt_DisableGlobalInterrupts();
		didDisableInterrupts = true;
	}
		
	// Copy the data into the ring buffer
	inRingBuffer->buffer[ inRingBuffer->headIndex ] = inData;
	
	_PlatformRingBuffer_UpdateHeadIndex( inRingBuffer, 1 );
	
	status = PlatformStatus_Success;
exit:
	// Enable global interrupts, if we disabled them
	if ( didDisableInterrupts )
	{
		PlatformInterrupt_EnableGlobalInterrupts();
	}
	return status;
}

PlatformStatus PlatformRingBuffer_ReadBuffer( PlatformRingBuffer *const inRingBuffer, 
										      uint8_t *const            outData, 
										      const size_t              inRequestedLen )
{
	PlatformStatus status = PlatformStatus_Failed;
	size_t sizeToCopy;
	size_t sizeCopied;
	
	bool didDisableInterrupts = false;
	
	require_quiet( inRingBuffer,   exit );
	require_quiet( outData,        exit );
	require_quiet( inRequestedLen, exit );
	
	// Disable Global Interrupts, if enabled
	if ( PlatformInterrupt_AreGlobalInterruptsEnabled() )
	{
		PlatformInterrupt_DisableGlobalInterrupts();
		didDisableInterrupts = true;
	}
		
	// Make sure we have received at least the amount of data requested
	require_quiet( inRequestedLen <= _PlatformRingBuffer_GetNumUsedBytes( inRingBuffer ), exit );

	// Copy the data into the output buffer, up to the wrap around 0.
	sizeToCopy = MIN( inRequestedLen, inRingBuffer->bufferSize - inRingBuffer->tailIndex );
	
	memcpy( outData, (void*)&inRingBuffer->buffer[ inRingBuffer->tailIndex ], sizeToCopy );
	
	sizeCopied = sizeToCopy;
	
	// If there is more data to copy after the ring buffer index wrapped to 0, then copy it from index 0 to the buffer at an offset of the previous copy.
	if ( sizeCopied < inRequestedLen )
	{
		sizeToCopy = inRequestedLen - sizeCopied;
		memcpy( &outData[ sizeCopied ], (void*)&inRingBuffer->buffer[0], sizeToCopy );
	}
	
	// Update the tail ptr
	_PlatformRingBuffer_UpdateTailIndex( inRingBuffer, inRequestedLen );
	
	status = PlatformStatus_Success;
exit:
	// Enable global interrupts if we disabled them
	if ( didDisableInterrupts )
	{
		PlatformInterrupt_EnableGlobalInterrupts();
	}

	return status;
}

//====================================//
//    Static Function Definitions     //
//====================================//

static inline size_t _PlatformRingBuffer_GetNumFreeBytes( PlatformRingBuffer *const inRingBuffer )
{
	size_t numUsedBytes = _PlatformRingBuffer_GetNumUsedBytes( inRingBuffer );
	
	// Return the buffer size (minus the extra byte needed to differentiate empty and full) minus the number of used bytes.
	return inRingBuffer->bufferSize - numUsedBytes - PLATFORM_RING_BUFFER_OVERHEAD_BYTES;
}

static inline size_t _PlatformRingBuffer_GetNumUsedBytes( PlatformRingBuffer *const inRingBuffer )
{		
	return (( inRingBuffer->headIndex - inRingBuffer->tailIndex + inRingBuffer->bufferSize ) % inRingBuffer->bufferSize );
}

static inline void _PlatformRingBuffer_UpdateHeadIndex( PlatformRingBuffer *const inRingBuffer, size_t inSizeToIncrease )
{
	inRingBuffer->headIndex = ( inRingBuffer->headIndex + inSizeToIncrease ) % inRingBuffer->bufferSize;
}

static inline void _PlatformRingBuffer_UpdateTailIndex( PlatformRingBuffer *const inRingBuffer, size_t inSizeToIncrease )
{
	inRingBuffer->tailIndex = ( inRingBuffer->tailIndex + inSizeToIncrease ) % inRingBuffer->bufferSize;
}