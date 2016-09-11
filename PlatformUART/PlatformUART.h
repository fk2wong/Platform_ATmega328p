/*
 * PlatformUART.h
 *
 * Created: 2016-09-07 5:27:49 PM
 *  Author: Felix
 */ 


#ifndef PLATFORMUART_H_
#define PLATFORMUART_H_

#include "PlatformStatus.h"
#include "PlatformRingBuffer.h"
#include <avr/io.h>

/*!
 *\brief    Initializes the UART.
 *
 *\details  This function will also internally disable the power save for the UART with PlatformPowerSave_PowerOn().
 *
 *\param    inBaudRate   - Baud Rate for UART communication. Higher speeds ( above 576,000 BAUD ) may have corrupted RX data, 
 *                       - since there is no DMA controller on the ATMega328p.
 *\param    inRingBuffer - Ring buffer for RX data, needed since the RX FIFO can only store up to 3 bytes.
 *
 *\return   PlatformStatus_Success if initialized successfully. PlatformStatus_Failed if anything failed.
 */
PlatformStatus PlatformUART_Init( uint32_t inBaudRate, PlatformRingBuffer *const inRingBuffer );

/*!
 *\brief    Transmits data over UART.
 *
 *\param    inBuffer    - Buffer holding data to write.
 *\param    inBufferLen - Length of data to write.
 *
 *\return   PlatformStatus_Success if data written successfully. PlatformStatus_Failed if anything failed.
 */
PlatformStatus PlatformUART_Transmit( uint8_t* inBuffer, size_t inBufferLen );

/*!
 *\brief    Receives data over UART.
 *
 *\param    outBuffer      - Buffer to store read data. Must be at least of length inRequestedLen.
 *\param    inRequestedLen - Length of data to write.
 *
 *\return   PlatformStatus_Success if data written successfully. PlatformStatus_Failed if anything failed.
 */
PlatformStatus PlatformUART_Receive( uint8_t* const outBuffer, size_t inRequestedLen );


#endif /* PLATFORMUART_H_ */