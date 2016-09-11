/*
 * PlatformPowerSave.h
 *
 * Created: 2016-09-10 7:00:27 PM
 *  Author: Felix
 */ 


#ifndef PLATFORMPOWERSAVE_H_
#define PLATFORMPOWERSAVE_H_

#include "PlatformStatus.h"
 
typedef enum 
{
	PlatformPowerSavePeripheral_ADC = 0,
	PlatformPowerSavePeripheral_USART,
	PlatformPowerSavePeripheral_SPI,
	PlatformPowerSavePeripheral_Timer0,
	PlatformPowerSavePeripheral_Timer1,
	PlatformPowerSavePeripheral_Timer2,
	PlatformPowerSavePeripheral_I2C,
	PlatformPowerSavePeripheral_MaxPeripherals,
} PlatformPowerSavePeripheral_t;

/*!
 *\brief    Enables power on to all peripherals. See enum PlatformPowerSavePeripheral_t for a list of peripherals. 
 *
 *\return   PlatformStatus_Success if successful. PlatformStatus_Failed if anything failed.
 */
PlatformStatus PlatformPowerSave_PowerOnAllPeripherals( void );

/*!
 *\brief    Disables power on to all peripherals. See enum PlatformPowerSavePeripheral_t for a list of peripherals. 
 *
 *\details  After this function is called on a peripheral, its clock will be disabled and it will be unusable until PlatformPowerSave_PowerOnDomain() is called.
 *
 *\return   PlatformStatus_Success if successful. PlatformStatus_Failed if anything failed.
 */
PlatformStatus PlatformPowerSave_PowerOffAllPeripherals( void );

/*!
 *\brief    Enables power on to a peripheral. See enum PlatformPowerSavePeripheral_t for a list of peripherals. 
 *
 *\return   PlatformStatus_Success if successful. PlatformStatus_Failed if anything failed.
 */
PlatformStatus PlatformPowerSave_PowerOnPeripheral( PlatformPowerSavePeripheral_t inDomain );

/*!
 *\brief    Disables power on to a peripheral. See enum PlatformPowerSavePeripheral_t for a list of peripherals. 
 *
 *\details  After this function is called on a peripheral, its clock will be disabled and it will be unusable until PlatformPowerSave_PowerOnDomain() is called.
 *
 *\return   PlatformStatus_Success if successful. PlatformStatus_Failed if anything failed.
 */
PlatformStatus PlatformPowerSave_PowerOffPeripheral( PlatformPowerSavePeripheral_t inDomain );



#endif /* PLATFORMPOWERSAVE_H_ */