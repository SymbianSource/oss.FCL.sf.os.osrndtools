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

#ifndef MEMSPYDRIVERLOGICALCHANHEAPINFO_H
#define MEMSPYDRIVERLOGICALCHANHEAPINFO_H

// System includes
#include <e32cmn.h>
#include <kern_priv.h>
#ifdef __MARM__
#include <arm.h>
#endif
#include <memspy/driver/memspydriverobjectsshared.h>
#include <memspy/driver/memspydriverenumerationsshared.h>

// User includes
#include "MemSpyDriverOpCodes.h"
#include "MemSpyDriverLogChanHeapBase.h"
#include "MemSpyDriverObjectsInternal.h"
#include "MemSpyDriverHeapWalker.h"
#include "MemSpyDriverEnumerationsInternal.h"

// Classes referenced
class DMemSpyDriverDevice;
class RMemSpyDriverRHeapKernel;
class RMemSpyMemStreamWriter;


class DMemSpyDriverLogChanHeapInfo : public DMemSpyDriverLogChanHeapBase
	{
public:
	DMemSpyDriverLogChanHeapInfo( DMemSpyDriverDevice& aDevice, DThread& aThread );
	~DMemSpyDriverLogChanHeapInfo();

private: // from DMemSpyDriverLogChanBase
	TInt Request( TInt aFunction, TAny* a1, TAny* a2 );
    TBool IsHandler( TInt aFunction ) const;

private: // Channel operation handlers
    TInt GetHeapInfoUser( TMemSpyDriverInternalHeapRequestParameters* aParams );
    TInt GetHeapInfoKernel( TMemSpyDriverInternalHeapRequestParameters* aParams, TDes8* aTransferBuffer );
    TInt GetIsDebugKernel(TAny* aIsDebugKernel);

private: // From MHeapWalkerObserver
    void HandleHeapWalkInit();
    TBool HandleHeapCell( TMemSpyDriverCellType aCellType, TAny* aCellAddress, TInt aLength, TInt aNestingLevel, TInt aAllocNumber );

private: // Internal methods
	void ReleaseCellList();
    TInt PrepareCellListTransferBuffer();
	TInt FetchCellList(TDes8* aBufferSink);
    TInt CalculateCellListBufferSize() const;

private: // Data members
    TMemSpyDriverInternalHeapRequestParameters iHeapInfoParams;
	RArray<TMemSpyDriverCell> iCellList;
    RMemSpyMemStreamWriter* iHeapStream;
	};


#endif
