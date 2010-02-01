/*
* Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies). 
* All rights reserved.
* This component and the accompanying materials are made available
* under the terms of "Eclipse Public License v1.0"
* which accompanies this distribution, and is available
* at the URL "http://www.eclipse.org/legal/epl-v10.html".
*
* Initial Contributors:
* Nokia Corporation - initial contribution.
*
* Contributors:
*
* Description:
*
*/

#ifndef MEMSPYDRIVERENUMERATIONSSHARED_H
#define MEMSPYDRIVERENUMERATIONSSHARED_H

// System includes
#include <u32std.h>

// Enumerations
enum TMemSpyDriverCellType
	{
	EMemSpyDriverGoodAllocatedCell = 0,
	EMemSpyDriverGoodFreeCell,
	EMemSpyDriverBadAllocatedCellSize,
	EMemSpyDriverBadAllocatedCellAddress,
	EMemSpyDriverBadFreeCellAddress,
	EMemSpyDriverBadFreeCellSize
	};


enum TMemSpyDriverThreadOrProcess
    {
    EMemSpyDriverThreadOrProcessTypeThread = EThread, // From TObjectType
    EMemSpyDriverThreadOrProcessTypeProcess = EProcess
    };


enum TMemSpyDriverDomainType
    {
    EMemSpyDriverDomainUser = 0,
    EMemSpyDriverDomainKernel
    };


enum TMemSpyDriverChunkType
    {
    EMemSpyDriverChunkTypeUnknown = 0,
    EMemSpyDriverChunkTypeHeap,
    EMemSpyDriverChunkTypeHeapKernel,
    EMemSpyDriverChunkTypeStackAndProcessGlobalData,
    EMemSpyDriverChunkTypeStackKernel,
    EMemSpyDriverChunkTypeGlobalData,
    EMemSpyDriverChunkTypeCode,
    EMemSpyDriverChunkTypeCodeGlobal,
    EMemSpyDriverChunkTypeCodeSelfModifiable,
    EMemSpyDriverChunkTypeLocal,
    EMemSpyDriverChunkTypeGlobal,
    EMemSpyDriverChunkTypeRamDrive
    };


enum TMemSpyDriverContainerType
    {
    EMemSpyDriverContainerTypeUnknown          = -1,
    EMemSpyDriverContainerTypeFirst            = EThread,
	EMemSpyDriverContainerTypeThread           = EMemSpyDriverContainerTypeFirst,
	EMemSpyDriverContainerTypeProcess          = EProcess,
	EMemSpyDriverContainerTypeChunk            = EChunk,
	EMemSpyDriverContainerTypeLibrary          = ELibrary,
	EMemSpyDriverContainerTypeSemaphore        = ESemaphore,
	EMemSpyDriverContainerTypeMutex            = EMutex,
	EMemSpyDriverContainerTypeTimer            = ETimer,
	EMemSpyDriverContainerTypeServer           = EServer,
	EMemSpyDriverContainerTypeSession          = ESession,
	EMemSpyDriverContainerTypeLogicalDevice    = ELogicalDevice,
	EMemSpyDriverContainerTypePhysicalDevice   = EPhysicalDevice,
	EMemSpyDriverContainerTypeLogicalChannel   = ELogicalChannel,
	EMemSpyDriverContainerTypeChangeNotifier   = EChangeNotifier,
	EMemSpyDriverContainerTypeUndertaker       = EUndertaker,
	EMemSpyDriverContainerTypeMsgQueue         = EMsgQueue,
	EMemSpyDriverContainerTypePropertyRef      = EPropertyRef,
	EMemSpyDriverContainerTypeCondVar          = ECondVar,
	EMemSpyDriverContainerTypeLast             = EMemSpyDriverContainerTypeCondVar
    };


enum TMemSpyDriverTimerType // Same as TTimer::TTimerType
    {
    EMemSpyDriverTimerTypeUnknown = 0,
	EMemSpyDriverTimerTypeRelative = 1,
    EMemSpyDriverTimerTypeAbsolute = 2,
    EMemSpyDriverTimerTypeLocked = 4,
    EMemSpyDriverTimerTypeHighRes = 8,
    EMemSpyDriverTimerTypeInactivity = 16
    };


enum TMemSpyDriverTimerState // Same as TTimer::TTimerState
    {
    EMemSpyDriverTimerStateUnknown = -1,
	EMemSpyDriverTimerStateIdle = 0,
    EMemSpyDriverTimerStateWaiting,
    EMemSpyDriverTimerStateWaitHighRes
    };

enum TMemSpyDriverEventType
    {
    EMemSpyDriverEventTypeThreadCreate = 0,
    EMemSpyDriverEventTypeThreadKill,
    EMemSpyDriverEventTypeProcessRemove,
    EMemSpyDriverEventTypeProcessCreate,
    EMemSpyDriverEventTypeChunkAdd,
    EMemSpyDriverEventTypeChunkDestroy,
    };

enum TMemSpyMemoryModelType
    {
    EMemSpyMemoryModelTypeUnknown = 0,
    EMemSpyMemoryModelTypeMoving,
    EMemSpyMemoryModelTypeMultiple,
    EMemSpyMemoryModelTypeEmulator
    };

enum TMemSpyChunkAttributes
	{
	EMemSpyChunkAttributesNormal            = 0x00,
	EMemSpyChunkAttributesDoubleEnded	    = 0x01,
	EMemSpyChunkAttributesDisconnected	    = 0x02,
	EMemSpyChunkAttributesConstructed   	= 0x04,
	EMemSpyChunkAttributesMemoryNotOwned	= 0x08
	};

#endif