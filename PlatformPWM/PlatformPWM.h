/*
 * PlatformPWM.h
 *
 * Created: 2016-09-24 3:47:14 PM
 *  Author: Felix
 */ 


#ifndef PLATFORMPWM_H_
#define PLATFORMPWM_H_

#include "PlatformStatus.h"
#include <stdint.h>

typedef enum
{
	PlatformPWM_0A, 
	PlatformPWM_0B,
	PlatformPWM_1A,
	PlatformPWM_1B,
	PlatformPWM_2A, 
	PlatformPWM_2B,
} PlatformPWM_t;

/*!
 *\brief    Initialize a specified PWM channel.
 *
 *\details  This function will perform the necessary steps to initialize and enable power to the PWM peripheral block.
 *          Some PWM channels share a timer (ie. PlatformPWM_0A and 0B, 1A and 1B, 2A and 2B ). These shared channels must have the same frequency;
 *          If the first one is initialized, the second one must be initialized with the same frequency (or at least one that produces the same outActualPWMFrequency).
 *
 *          Note: The PWM frequency will be selected on a best-match basis, and the actual frequency will be returned in outActualPWMFrequency.
 *
 *\param    inPWM                   - PWM channel to initialize.
 *\param    inRequestedPWMFrequency - Requested PWM Frequency for this channel.
 *\param    outActualPWMFrequency   - Actual PWM Frequency that was set. This will be the returned as the closest match to the inRequestedPWMFrequency. 
 *
 *\return   PlatformStatus - PlatformStatus_Success            if successful,
 *                         - PlatformStatus_AlreadyInitialized if this channel has already been initialized,
 *                         - PlatformStatus_Failed             if anything else failed.
 */
PlatformStatus PlatformPWM_Init( const PlatformPWM_t inPWM, const uint32_t inRequestedPWMFrequency, uint32_t *const outActualPWMFrequency );

/*!
 *\brief    Start PWM output on a specified channel.
 *
 *\param    inPWM       - PWM channel.
 *\param    inDutyCycle - Duty cycle of the PWM to output, from 0.0 to 100.0.
 *
 *\return   PlatformStatus - PlatformStatus_Success        if successful,
 *                         - PlatformStatus_NotInitialized if this channel has not been initialized,
 *                         - PlatformStatus_Failed         if anything else failed.
 */
PlatformStatus PlatformPWM_Start( const PlatformPWM_t inPWM, const float inDutyCycle );

/*!
 *\brief    Stop PWM output on a specified channel.
 *
 *\param    inPWM - PWM channel.
 *
 *\return   PlatformStatus - PlatformStatus_Success        if successful,
 *                         - PlatformStatus_NotInitialized if this channel has not been initialized,
 *                         - PlatformStatus_Failed         if anything else failed.
 */
PlatformStatus PlatformPWM_Stop( const PlatformPWM_t inPWM );

/*!
 *\brief    Deinitialize a PWM channel.
 *
 *\param    inPWM - PWM Channel to deinitialize.
 *
 *\return   PlatformStatus - PlatformStatus_Success        if successful,
 *                         - PlatformStatus_NotInitialized if this channel has not been initialized,
 *                         - PlatformStatus_Failed         if anything else failed.
 */
PlatformStatus PlatformPWM_Deinit( const PlatformPWM_t inPWM );

#endif /* PLATFORMPWM_H_ */