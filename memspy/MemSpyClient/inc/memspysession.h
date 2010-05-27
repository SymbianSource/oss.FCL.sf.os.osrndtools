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


#ifndef MEMSPYSESSION_H
#define MEMSPYSESSION_H


// System includes
#include <e32base.h>
#include <u32std.h>

//user includes
#include <memspy/engine/memspyprocessdata.h> //for Processes
#include <memspy/api/memspyapiprocess.h>

#include <memspy/engine/memspythreaddata.h> //for Threads
#include <memspy/api/memspyapithread.h>
#include <memspy/api/memspyapithreadinfoitem.h>
#include <memspy/engine/memspythreadinfoitemdata.h>
#include <memspy/engine/memspyengineobjectthreadinfoobjects.h>

#include <memspy/engine/memspykernelobjectdata.h> //for KernelObjects
#include <memspy/api/memspyapikernelobject.h>

#include <memspy/api/memspyapikernelobjectitem.h> //for KernelObjectItems

#include <memspy/engine/memspyheapdata.h> //for Heap
#include <memspy/api/memspyapiheap.h>

#include <memspyengineclientinterface.h>

// Constants
const TInt KMemSpyVersion           = 2;

enum TSortType
	{
	ESortProcById,
	ESortProcByName,
	ESortProcByThreadCount,
	ESortProcByCodeSegs,
	ESortProcByHeapUsage,
	ESortProcByStackUsage
	};

enum TMemSpyOutputType
    {
    EOutputTypeDebug = 0,
    EOutputTypeFile
    };


NONSHARABLE_CLASS( RMemSpySession ) : public RSessionBase
    {
public:
    IMPORT_C RMemSpySession();
    IMPORT_C TInt Connect();
    
public:	//API
    IMPORT_C void OutputKernelHeapDataL(TMemSpyOutputType aOutputType); //EMemSpyClientServerOpHeapData
    IMPORT_C void OutputThreadHeapDataL(TMemSpyOutputType aOutputType, TThreadId aThreadId); //EMemSpyClientServerOpHeapData
    IMPORT_C void OutputThreadCellListL(TMemSpyOutputType aOutputType, TThreadId aThreadId);//EMemSpyClientServerOpHeapCellListing
    IMPORT_C void OutputKernelObjectsL(TMemSpyOutputType aOutputType);// EMemSpyClientServerOpEnumerateKernelContainerAll
    IMPORT_C void OutputCompactStackInfoL(TMemSpyOutputType aOutputType);// EMemSpyClientServerOpStackInfoCompact
    IMPORT_C void OutputCompactHeapInfoL(TMemSpyOutputType aOutputType);// EMemSpyClientServerOpHeapInfoCompact
    
    IMPORT_C void GetProcessesL(RArray<CMemSpyApiProcess*> &aProcesses, TSortType aSortType = ESortProcById);
    
    IMPORT_C TProcessId GetProcessIdByNameL(const TDesC& aProcessName);
    
    IMPORT_C void GetThreadsL(TProcessId aProcessId, RArray<CMemSpyApiThread*> &aThreads, TSortType aSortType = ESortProcById);
    
    IMPORT_C TInt ProcessSystemPermanentOrCritical( TProcessId aId, TBool aValue ); //aValue -> return value
    
    IMPORT_C void SetThreadPriorityL(TThreadId aId, TInt aPriority);
    IMPORT_C TInt EndProcess( TProcessId aId, TMemSpyEndType aType );
    
    IMPORT_C TInt SwitchToProcess( TProcessId aId, TBool aBrought  );
    
    
    //Threads operations
    /**
     * 
     */
    IMPORT_C void GetThreadsL(TProcessId aProcessId, RArray<CMemSpyApiProcess*> &aThreads, TSortType aSortType = ESortProcById);
    
    IMPORT_C TInt ThreadSystemPermanentOrCritical( TThreadId aId, TBool aValue );
        
    IMPORT_C TInt EndThread( TThreadId aId, TMemSpyEndType aType );
    
    IMPORT_C TInt SwitchToThread( TThreadId aId, TBool aBrought );
    
    IMPORT_C TInt GetInfoItemType( TInt aIndex, TThreadId aId, TMemSpyThreadInfoItemType &aType );
    
    IMPORT_C void GetThreadInfoItems( RArray<CMemSpyApiThreadInfoItem*> &aInfoItems, TThreadId aId, TMemSpyThreadInfoItemType aType );
    
    //KernelObjects operations    
    /**
     * 
     */
    IMPORT_C TInt GetKernelObjects( RArray<CMemSpyApiKernelObject*> &aKernelObjects );
    
    IMPORT_C TInt GetKernelObjectItems( RArray<CMemSpyApiKernelObjectItem*> &aKernelObjectItems, TMemSpyDriverContainerType aForContainer );
    
    IMPORT_C void OutputAllContainerContents();
    
    /**
     * 
     */    
	IMPORT_C CMemSpyApiHeap* GetHeap();
	
	IMPORT_C void DumpKernelHeap();
    
private:
    TInt StartServer();
    
    void SetOutputTypeL(TMemSpyOutputType aOutputType);
    };

#endif // MEMSPYSESSION_H
