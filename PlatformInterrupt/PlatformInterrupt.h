/*
 * PlatformInterrupt.h
 *
 * Created: 2016-09-10 6:00:31 PM
 *  Author: Felix
 */ 


#ifndef PLATFORMINTERRUPT_H_
#define PLATFORMINTERRUPT_H_

#include <avr/interrupt.h>

#define PlatformInterrupt_AreGlobalInterruptsEnabled() ( SREG & ( 1 << SREG_I ))
#define PlatformInterrupt_EnableGlobalInterrupts()     sei()
#define PlatformInterrupt_DisableGlobalInterrupts()    cli()

#endif /* PLATFORMINTERRUPT_H_ */