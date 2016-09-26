/*
 * PlatformADC.h
 *
 * Created: 2016-09-11 11:08:16 PM
 *  Author: Felix
 */ 


#ifndef PLATFORMADC_H_
#define PLATFORMADC_H_

#include "PlatformStatus.h"
#include <stdint.h>

typedef enum 
{
	PlatformADC_ADC0,
	PlatformADC_ADC1,
	PlatformADC_ADC2,
	PlatformADC_ADC3,
	PlatformADC_ADC4,
	PlatformADC_ADC5,
	PlatformADC_Max,
} PlatformADC_t;

/*!
 *\brief    Initializes the ADC for a specified input. Should be called before calling PlatformADC_Read().
 *
 *\details  This will enable the ADC input as well as its input buffer. 
 *          If this is the first ADC to be initialized, this function will perform the necessary steps 
 *          to initialize and enable power to the entire ADC peripheral block on the chip.
 *
 *\param    inADC - The ADC input to initialize.
 *
 *\return   PlatformStatus_Success if successful. PlatformStatus_Failed if anything failed.
 */
PlatformStatus PlatformADC_Init( PlatformADC_t inADC );

/*!
 *\brief    Reads the ADC on a selected input.
 *
 *\param    inADC       - ADC input to read.
 *\param    outADCValue - Pointer to store the ADC value read. This value adheres to the formula:
 *                        outADCValue = ( VIn * 1024 ) / VRef 
 *
 *\return   PlatformStatus_Success if successful. PlatformStatus_Failed if anything failed.
 */
PlatformStatus PlatformADC_Read( PlatformADC_t inADC, uint16_t *const outADCValue );

/*!
 *\brief    Deinitializes the ADC for a specified input.
 *
 *\details  This will disable the ADC input as well as its input buffer.
 *          If there are no more initialized ADC inputs after this, this function will perform the necessary steps
 *          to deinitialize and disable power to the entire ADC peripheral block on the chip.
 *
 *\return   PlatformStatus_Success if successful. PlatformStatus_Failed if anything failed.
 */
PlatformStatus PlatformADC_Deinit( PlatformADC_t inADC );

/*!
 *\brief    Disables all input buffers for the ADC inputs. 
 *
 *\details  This reduces power consumption for unused ADC inputs, and should be called during program initialization.
 *          The input buffer will be re-enabled when calling PlatformADC_Init() for the specific ADC input.
 *          This ensures that any non-used ADC inputs will not be consuming power.
 *
 *\return   PlatformStatus_Success if successful. PlatformStatus_Failed if anything failed.
 */
PlatformStatus PlatformADC_DisableAllInputBuffers( void );

#endif /* PLATFORMADC_H_ */