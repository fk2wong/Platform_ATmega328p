/*
 * PlatformGPIO.c
 *
 * Created: 2016-09-06 12:31:18 PM
 *  Author: Felix
 */ 

#include "PlatformGPIO.h"
#include "require_macros.h"
#include <stdint.h>

//================//
//    Defines     //
//================//

#define PLATFORM_GPIO_REG_BASE   ( 0x20 )
#define PLATFORM_GPIO_REG_OFFSET ( 0x03 )

#define PLATFORM_GPIO_GET_REG_GROUP_FROM_PORT_NAME( PORT ) ( PlatformPortRegGroup_t* )( PLATFORM_GPIO_REG_BASE + ( PORT ) * PLATFORM_GPIO_REG_OFFSET )
#define PLATFORM_GPIO_GET_PIN_MASK( PIN )                  ( uint8_t )( 1 << ( PIN ))

//================//
//    Typedefs    //
//================//

typedef struct 
{
	volatile uint8_t PINCTL;
	volatile uint8_t DIR;
	volatile uint8_t PORTDATA;
} PlatformPortRegGroup_t;

typedef enum
{
	PlatformPortA = 0,
	PlatformPortB = 1,
	PlatformPortC = 2,
	PlatformPortD = 3,
} PlatformPort_t;

typedef enum
{
	PlatformPin0,
	PlatformPin1,
	PlatformPin2,
	PlatformPin3,
	PlatformPin4,
	PlatformPin5,
	PlatformPin6,
	PlatformPin7,
} PlatformPin_t;

typedef struct
{
	uint8_t              portName;           // See iom328p.h for port defines
	uint8_t              pin;            // See iom328p.h for pin defines
	PlatformGPIOConfig_t initialConfig;  // See PlatformGPIO.h
} PlatformGPIOStruct_t;

//===========================//
//    Structs & Variables    //
//===========================//

static const PlatformGPIOStruct_t platformGPIOList[] =
{
	{
		.portName      = PlatformPortD,
		.pin           = PlatformPin2,
		.initialConfig = PlatformGPIOConfig_InputPullUp,
	},
	{
		.portName      = PlatformPortD,
		.pin           = PlatformPin3,
		.initialConfig = PlatformGPIOConfig_InputPullUp,
	},
	{
		.portName      = PlatformPortD,
		.pin           = PlatformPin4,
		.initialConfig = PlatformGPIOConfig_InputPullUp,
	},
	{
		.portName      = PlatformPortD,
		.pin           = PlatformPin5,
		.initialConfig = PlatformGPIOConfig_InputPullUp,
	},
	{
		.portName      = PlatformPortD,
		.pin           = PlatformPin6,
		.initialConfig = PlatformGPIOConfig_InputPullUp,
	},
	{
		.portName      = PlatformPortD,
		.pin           = PlatformPin7,
		.initialConfig = PlatformGPIOConfig_InputPullUp,
	},
	{
		.portName      = PlatformPortC,
		.pin           = PlatformPin0,
		.initialConfig = PlatformGPIOConfig_InputPullUp,
	},
	{
		.portName      = PlatformPortC,
		.pin           = PlatformPin1,
		.initialConfig = PlatformGPIOConfig_InputPullUp,
	},
	{
		.portName      = PlatformPortB,
		.pin           = PlatformPin0,
		.initialConfig = PlatformGPIOConfig_InputPullUp,
	},
	{
		.portName      = PlatformPortB,
		.pin           = PlatformPin1,
		.initialConfig = PlatformGPIOConfig_InputPullUp,
	},
	{
		.portName      = PlatformPortB,
		.pin           = PlatformPin2,
		.initialConfig = PlatformGPIOConfig_InputPullUp,
	},
	{
		.portName      = PlatformPortB,
		.pin           = PlatformPin3,
		.initialConfig = PlatformGPIOConfig_InputPullUp,
	},
	{
		.portName      = PlatformPortB,
		.pin           = PlatformPin4,
		.initialConfig = PlatformGPIOConfig_InputPullUp,
	},	
	{
		.portName      = PlatformPortB,
		.pin           = PlatformPin5,
		.initialConfig = PlatformGPIOConfig_Output,
	},
};

//====================================//
//    Static Function Declarations    //
//====================================//

static PlatformStatus _PlatformGPIO_Configure(  PlatformPortRegGroup_t* inRegGroup, uint8_t inPin, PlatformGPIOConfig_t inConfig );
static PlatformStatus _PlatformGPIO_OutputHigh( PlatformPortRegGroup_t* inRegGroup, uint8_t inPin );
static PlatformStatus _PlatformGPIO_OutputLow(  PlatformPortRegGroup_t* inRegGroup, uint8_t inPin );
static PlatformStatus _PlatformGPIO_Toggle(     PlatformPortRegGroup_t* inRegGroup, uint8_t inPin );
static PlatformStatus _PlatformGPIO_GetInput(   PlatformPortRegGroup_t* inRegGroup, uint8_t inPin, bool *const outLogicLevel );

static inline bool _PlatformGPIO_IsConfiguredAsOutput( PlatformPortRegGroup_t* inRegGroup, uint8_t inPin );

//====================================//
//    Public Function Definitions     //
//====================================//

PlatformStatus PlatformGPIO_InitAllGPIOs( void )
{
	PlatformStatus status;
	
	for ( uint8_t i = 0; i < ( sizeof( platformGPIOList ) / sizeof( PlatformGPIOStruct_t )); i++ )
	{
		const PlatformGPIOStruct_t *const gpioStruct = &platformGPIOList[ i ];
		
		status = _PlatformGPIO_Configure( PLATFORM_GPIO_GET_REG_GROUP_FROM_PORT_NAME( gpioStruct->portName ), gpioStruct->pin, gpioStruct->initialConfig );
		require_noerr_quiet( status, exit );
	}
	
exit:
	return status;
}

PlatformStatus PlatformGPIO_Configure( PlatformGPIO_t inGPIO, PlatformGPIOConfig_t inConfig )
{
	const PlatformGPIOStruct_t *const gpioStruct = &platformGPIOList[ inGPIO ];
	
	return _PlatformGPIO_Configure( PLATFORM_GPIO_GET_REG_GROUP_FROM_PORT_NAME( gpioStruct->portName ), gpioStruct->pin, inConfig );
}

PlatformStatus PlatformGPIO_OutputHigh( PlatformGPIO_t inGPIO )
{
	const PlatformGPIOStruct_t *const gpioStruct = &platformGPIOList[ inGPIO ];
	
	return _PlatformGPIO_OutputHigh( PLATFORM_GPIO_GET_REG_GROUP_FROM_PORT_NAME( gpioStruct->portName ), gpioStruct->pin );
}

PlatformStatus PlatformGPIO_OutputLow( PlatformGPIO_t inGPIO )
{
	const PlatformGPIOStruct_t *const gpioStruct = &platformGPIOList[ inGPIO ];
	
	return _PlatformGPIO_OutputLow( PLATFORM_GPIO_GET_REG_GROUP_FROM_PORT_NAME( gpioStruct->portName ), gpioStruct->pin );
}

PlatformStatus PlatformGPIO_Toggle( PlatformGPIO_t inGPIO )
{
	const PlatformGPIOStruct_t *const gpioStruct = &platformGPIOList[ inGPIO ];
	
	return _PlatformGPIO_Toggle( PLATFORM_GPIO_GET_REG_GROUP_FROM_PORT_NAME( gpioStruct->portName ), gpioStruct->pin );
}

PlatformStatus PlatformGPIO_GetInput( PlatformGPIO_t inGPIO, bool *const outLogicLevel )
{
	const PlatformGPIOStruct_t *const gpioStruct = &platformGPIOList[ inGPIO ];
	
	return _PlatformGPIO_GetInput( PLATFORM_GPIO_GET_REG_GROUP_FROM_PORT_NAME( gpioStruct->portName ), gpioStruct->pin, outLogicLevel );
}

//====================================//
//    Static Function Definitions     //
//====================================//

static PlatformStatus _PlatformGPIO_Configure( PlatformPortRegGroup_t* inRegGroup, uint8_t inPin, PlatformGPIOConfig_t inConfig )
{
	PlatformStatus status = PlatformStatus_Failed;
	
	require_quiet( inRegGroup, exit );
	
	switch ( inConfig )
	{
		case PlatformGPIOConfig_Output:
		{
			// Set the set as tri-state before setting the direction, so we drive low by default
			inRegGroup->PORTDATA &= ~PLATFORM_GPIO_GET_PIN_MASK( inPin );
			inRegGroup->DIR      |= PLATFORM_GPIO_GET_PIN_MASK( inPin );
			break;
		}
		case PlatformGPIOConfig_InputHighZ:
		{
			// Set the direction as input pull-up before tri-stating, so we don't drive low
			inRegGroup->DIR      &= ~PLATFORM_GPIO_GET_PIN_MASK( inPin );
			inRegGroup->PORTDATA &= ~PLATFORM_GPIO_GET_PIN_MASK( inPin );
			break;
		}
		case PlatformGPIOConfig_InputPullUp:
		{
			// Set as tri-state input before setting as pull up, so we don't drive high
			inRegGroup->DIR      &= ~PLATFORM_GPIO_GET_PIN_MASK( inPin );
			inRegGroup->PORTDATA |= PLATFORM_GPIO_GET_PIN_MASK( inPin );
		}
		default:
		{
			break;
		}
	}
	
	status = PlatformStatus_Success;
exit:
	return status;
}

static PlatformStatus _PlatformGPIO_OutputHigh( PlatformPortRegGroup_t* inRegGroup, uint8_t inPin )
{
	PlatformStatus status = PlatformStatus_Failed;
	bool isConfiguredAsOutput;
		
	require_quiet( inRegGroup, exit );
		
	// Verify the GPIO is configured as output
	isConfiguredAsOutput = _PlatformGPIO_IsConfiguredAsOutput( inRegGroup, inPin );
	require_quiet( isConfiguredAsOutput, exit );
		
	inRegGroup->PORTDATA |= PLATFORM_GPIO_GET_PIN_MASK( inPin );
	
	status = PlatformStatus_Success;
exit:
	return status;
}

static PlatformStatus _PlatformGPIO_OutputLow( PlatformPortRegGroup_t* inRegGroup, uint8_t inPin )
{
	PlatformStatus status = PlatformStatus_Failed;
	bool isConfiguredAsOutput;
		
	require_quiet( inRegGroup, exit );
		
	// Verify the GPIO is configured as output
	isConfiguredAsOutput = _PlatformGPIO_IsConfiguredAsOutput( inRegGroup, inPin );
	require_quiet( isConfiguredAsOutput, exit );
		
	inRegGroup->PORTDATA &= ~PLATFORM_GPIO_GET_PIN_MASK( inPin );
	
	status = PlatformStatus_Success;
exit:
	return status;
}

static PlatformStatus _PlatformGPIO_Toggle( PlatformPortRegGroup_t* inRegGroup, uint8_t inPin )
{
	PlatformStatus status = PlatformStatus_Failed;
	bool isConfiguredAsOutput;
		
	require_quiet( inRegGroup, exit );
	
	// Verify the GPIO is configured as output
	isConfiguredAsOutput = _PlatformGPIO_IsConfiguredAsOutput( inRegGroup, inPin );
	require_quiet( isConfiguredAsOutput, exit );
	
	inRegGroup->PORTDATA ^= PLATFORM_GPIO_GET_PIN_MASK( inPin );
	
	status = PlatformStatus_Success;
exit:
	return status;
}

static PlatformStatus _PlatformGPIO_GetInput( PlatformPortRegGroup_t* inRegGroup, uint8_t inPin, bool *const outLogicLevel )
{
	PlatformStatus status = PlatformStatus_Failed;
	bool isConfiguredAsOutput;
	
	require_quiet( inRegGroup,    exit );
	require_quiet( outLogicLevel, exit );
	
	// Verify the GPIO is configured as input
	isConfiguredAsOutput = _PlatformGPIO_IsConfiguredAsOutput( inRegGroup, inPin );
	require_quiet( !isConfiguredAsOutput, exit );
	
	// Get the intput level
	*outLogicLevel = inRegGroup->PINCTL & PLATFORM_GPIO_GET_PIN_MASK( inPin );
	
	status = PlatformStatus_Success;
exit:
	return status;
}

static inline bool _PlatformGPIO_IsConfiguredAsOutput( PlatformPortRegGroup_t* inRegGroup, uint8_t inPin )
{
	return ( inRegGroup->DIR & PLATFORM_GPIO_GET_PIN_MASK( inPin )) ? true : false;
}