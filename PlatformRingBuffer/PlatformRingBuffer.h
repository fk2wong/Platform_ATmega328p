/*
 * PlatformRingBuffer.h
 *
 * Created: 2016-09-08 7:57:11 AM
 *  Author: Felix
 */ 


#ifndef PLATFORMRINGBUFFER_H_
#define PLATFORMRINGBUFFER_H_

#include "PlatformStatus.h"
#include <avr/io.h>
#include <stddef.h>

#define PLATFORM_RING_BUFFER_MAX_SIZE ( 512 )

typedef struct PlatformRingBufferStruct PlatformRingBuffer;

/*!
 *\brief    Creates a ring buffer of specified size.
 *
 *\param    inBufferSize - Size of the ring buffer to create.
 *
 *\return   PlatformRingBuffer* - Pointer to ring buffer object created.
 */
PlatformRingBuffer * PlatformRingBuffer_Create( size_t inBufferSize );

/*!
 *\brief    Writes to the ring buffer from a source buffer.
 *
 *\param    inRingBuffer - Ring buffer to copy data into.
 *\param    inData       - Data buffer to copy from.
 *\param    inDataLen    - Length of data to copy.
 *
 *\return   PlatformStatus_Success if written successfully. PlatformStatus_Failed if anything failed.
 */
PlatformStatus PlatformRingBuffer_WriteBuffer( PlatformRingBuffer *const inRingBuffer, 
											   const uint8_t *const      inData, 
											   size_t                    inDataLen );
/*!
 *\brief    Writes a byte to the ring buffer.
 *
 *\param    inRingBuffer - Ring buffer to write to.
 *\param    inData       - Data byte to be written.
 *
 *\return   PlatformStatus_Success if written successfully. PlatformStatus_Failed if anything failed.
 */
								 
PlatformStatus PlatformRingBuffer_WriteByte( PlatformRingBuffer* const inRingBuffer, const uint8_t inData );

/*!
 *\brief    Reads from the ring buffer.
 *
 *\param    inRingBuffer   - Ring buffer to read from.
 *\param    outData        - Data buffer that will hold the data read from the ring buffer. Must be at least of size inRequestedLen; data will not be allocated.
 *\param    inRequestedLen - Length of data to read.
 *
 *\return   PlatformStatus_Success if read successfully. PlatformStatus_Failed if anything failed.
 */
PlatformStatus PlatformRingBuffer_ReadBuffer( PlatformRingBuffer *const inRingBuffer,
											  uint8_t *const            outData,
											  const size_t              inRequestedLen );

#endif /* PLATFORMRINGBUFFER_H_ */