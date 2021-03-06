/*
 * PlatformStatus.h
 *
 * Created: 2016-09-07 4:34:24 PM
 *  Author: Felix
 */ 


#ifndef PLATFORMSTATUS_H_
#define PLATFORMSTATUS_H_

typedef enum 
{
	PlatformStatus_Success = 0,
	PlatformStatus_Failed,
	PlatformStatus_NotInitialized,
	PlatformStatus_AlreadyInitialized,
	PlatformStatus_NotSupported,
	PlatformStatus_InvalidArgument,
} PlatformStatus;


#endif /* PLATFORMSTATUS_H_ */