/*
 * PlatformI2C.h
 *
 * Created: 2016-09-26 11:39:46 PM
 *  Author: Felix
 */ 


#ifndef PLATFORMI2C_H_
#define PLATFORMI2C_H_

#include "PlatformStatus.h"
#include <stdint.h>
#include <stddef.h>

/*!
 *\brief    Initializes I2C as fast mode (400kHz). 
 *
 *\details  This function will enable power to the I2C peripheral block and any necessary configurations.
 *          This will also override previous configurations on the SDA/SCL pins.
 *
 *\return   PlatformStatus - PlatformStatus_Success if successfully initialized. 
 *                         - PlatformStatus_AlreadyInitialized if I2C has already been initialized.
 *                         - PlatformStatus_Failed if anything else failed.
 */
PlatformStatus PlatformI2C_Init( void );

/*!
 *\brief    Writes one byte to an I2C device.
 *
 *\param    inDeviceAddr   - Address of the I2C device to write to.
 *\param    inRegisterAddr - Internal register address of the I2C device to write to.
 *\param    inDataByte     - Byte to write.
 *
 *\return   PlatformStatus - PlatformStatus_Success if the data was successfully written. 
                           - PlatformStatus_NotInitialized if I2C has not yet been initialized.
                           - PlatformStatus_Failed if anything else failed.
 */
PlatformStatus PlatformI2C_WriteByte( const uint8_t inDeviceAddr, const uint8_t inRegisterAddress, const uint8_t inDataByte );

/*!
 *\brief    Writes data to an I2C device.
 *
 *\param    inDeviceAddr   - Address of the I2C device to write to.
 *\param    inRegisterAddr - Internal register address of the I2C device to write to.
 *\param    inData         - Buffer containing write data.
 *\param    inDataLen      - Number of bytes to write.
 *
 *\return   PlatformStatus - PlatformStatus_Success if the data was successfully written. 
                           - PlatformStatus_NotInitialized if I2C has not yet been initialized.
                           - PlatformStatus_Failed if anything else failed.
 */
PlatformStatus PlatformI2C_Write( const uint8_t inDeviceAddr, const uint8_t inRegisterAddress, const uint8_t *const inData, const size_t inDataLen );

/*!
 *\brief    Reads data from an I2C device.
 *
 *\param    inDeviceAddr   - Address of the I2C device to read from.
 *\param    inRegisterAddr - Internal register address of the I2C device to read.
 *\param    outData        - Buffer to store read data.
 *\param    inDataLen      - Number of bytes to read.
 *
 *\return   PlatformStatus - PlatformStatus_Success if the data was read successfully.
                           - PlatformStatus_NotInitialized if I2C has not yet been initialized.
                           - PlatformStatus_Failed if anything else failed.
 */
PlatformStatus PlatformI2C_Read( const uint8_t inDeviceAddr, const uint8_t inRegisterAddress, uint8_t *const outData, const size_t inDataLen );

/*!
 *\brief    Deinitializes the I2C peripheral.
 *
 *\return   PlatformStatus - PlatformStatus_Success if deinitialized successfully. 
                           - PlatformStatus_NotInitialized if I2C has not yet been initialized.
                           - PlatformStatus_Failed if anything else failed.
 */
PlatformStatus PlatformI2C_Deinit( void );

#endif /* PLATFORMI2C_H_ */