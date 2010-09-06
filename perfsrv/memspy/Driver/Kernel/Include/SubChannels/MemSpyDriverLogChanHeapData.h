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

#ifndef MEMSPYDRIVERLOGICALCHANHEAPDATA_H
#define MEMSPYDRIVERLOGICALCHANHEAPDATA_H

// System includes
#include <e32cmn.h>
#include <kern_priv.h>
#ifdef __MARM__
#include <arm.h>
#endif
#include <memspy/driver/memspydriverobjectsshared.h>
#include <memspy/driver/memspydriverenumerationsshared.h>

// User includes
#include "MemSpyDriverHeap.h"
#include "MemSpyDriverOpCodes.h"
#include "MemSpyDriverLogChanHeapBase.h"
#include "MemSpyDriverObjectsInternal.h"
#include "MemSpyDriverEnumerationsInternal.h"

// Classes referenced
class DMemSpyDriverDevice;



class DMemSpyDriverLogChanHeapData : public DMemSpyDriverLogChanHeapBase
	{
public:
	DMemSpyDriverLogChanHeapData( DMemSpyDriverDevice& aDevice, DThread& aThread );
	~DMemSpyDriverLogChanHeapData();

private: // from DMemSpyDriverLogChanBase
	TInt Request( TInt aFunction, TAny* a1, TAny* a2 );
    TBool IsHandler( TInt aFunction ) const;

private: // Channel operation handlers
    TInt GetHeapDataUser( TMemSpyDriverInternalHeapDataParams* aParams );
        
private: // Internal methods
    TInt GetHeapDataUser( TMemSpyDriverInternalHeapDataParams& aParams );
    TInt GetHeapDataKernelInit( TMemSpyHeapInfo* aInfo, TDes8* aFreeCells );
    TInt GetHeapDataKernelFetch( TDes8* aSink );

private:
    TMemSpyDriverInternalHeapRequestParameters iHeapInfoParams;
    RMemSpyDriverRHeapKernelFromCopy iKernelHeap;
	};


#endif
