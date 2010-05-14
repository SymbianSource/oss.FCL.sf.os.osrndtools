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
#include <memspy/engine/memspykernelobjectdata.h> //for KernelObjects
#include <memspy/api/memspyapikernelobject.h>
#include <memspy/api/memspyapikernelobjectitem.h> //for KernelObjectItems
#include <memspy/engine/memspyheapdata.h> //for Heap
#include <memspy/api/memspyapiheap.h>

// Constants
const TInt KMemSpyVersion           = 2;

// Literal constants 
_LIT( KMemSpyServer2, "MemSpyServer.exe" );


//
enum TMemSpyOperations
    {
	//Output operations
	
//	EGetSinkType,
//	EOutputToDebug,
//	EOutputToFile,
//	EOutputKernelObjects,
//	ESetServerTimeOutStatus,
//	EOutputProcessInfo,
	
	//Processes operations
	
	EGetProcessCount,
	EGetProcesses,
	EGetThreadCount,
	EGetThreads,
	ESetThreadPriority,
	EProcessSystemPermanentOrCritical,
	EEndProcess,
	ESwitchToProcess,
//	EOpenCloseCurrentProcess
	
	//Threads operations
	EThreadSystemPermanentOrCritical,	
	EEndThread,
	ESwitchToThread,
	
	//Kernel Objects operations
	EGetKernelObjectTypeCount,
	EGetKernelObjects,
	EGetKernelObjectItemsCount,
	EGetKernelObjectItems,
	//EOutputKernelObjects
	
	//Heap operations
	EGetHeap
    };

enum TSortType
	{
	ESortProcById,
	ESortProcByName,
	ESortProcByThreadCount,
	ESortProcByCodeSegs,
	ESortProcByHeapUsage,
	ESortProcByStackUsage
	};

enum TEndType
	{
	ETerminate,
	EKill,
	EPanic
};

NONSHARABLE_CLASS( RMemSpySession ) : public RSessionBase
    {
public:
    IMPORT_C RMemSpySession();
    IMPORT_C TInt Connect();
    
public:	//API
    //Output operations
    //inline TMemSpySinkType GetSinkType();    
    //inline void OutputToDebug();
    //inline void OutputToFile();    
    //inline void OutputKernelObjects();    
    //inline void SetServerTimeOutStatus( TUint32 aValue, TBool aEnabled );
    //inline void OutputProcessInfo( TMemSpyProcess aProcess );
    
    //Processes operations    
    /**
     * 
     */
    IMPORT_C void GetProcessesL(RArray<CMemSpyApiProcess*> &aProcesses, TSortType aSortType = ESortProcById);        
    
    IMPORT_C void GetThreadsL(TProcessId aProcessId, RArray<CMemSpyApiThread*> &aThreads, TSortType aSortType = ESortProcById);
    IMPORT_C TInt ProcessSystemPermanentOrCritical( TProcessId aId, TBool aValue ); //aValue -> return value
    
    IMPORT_C void SetThreadPriorityL(TThreadId aId, TInt aPriority);
    IMPORT_C TInt EndProcess( TProcessId aId, TEndType aType );
    
    IMPORT_C TInt SwitchToProcess( TProcessId aId, TBool aBrought  );
    
    //inline void OpenCloseCurrentProcess( TProcessId aId, TBool aOpen );
    
    //Threads operations
    /**
     * 
     */
    IMPORT_C void GetThreadsL(TProcessId aProcessId, RArray<CMemSpyApiProcess*> &aThreads, TSortType aSortType = ESortProcById);
    
    IMPORT_C TInt ThreadSystemPermanentOrCritical( TThreadId aId, TBool aValue );
        
    IMPORT_C TInt EndThread( TThreadId aId, TEndType aType );
    
    IMPORT_C TInt SwitchToThread( TThreadId aId, TBool aBrought );
    
    //KernelObjects operations    
    /**
     * 
     */
    IMPORT_C TInt GetKernelObjects( RArray<CMemSpyApiKernelObject*> &aKernelObjects );
    
    IMPORT_C TInt GetKernelObjectItems( RArray<CMemSpyApiKernelObjectItem*> &aKernelObjectItems, TMemSpyDriverContainerType aForContainer );
    
    /**
     * 
     */
    //IMPORT_C TInt GetHeap( RArray<CMemSpyApiHeap*> &aHeap );
	//IMPORT_C TInt GetHeap( CMemSpyApiHeap* aHeap );
	IMPORT_C CMemSpyApiHeap* GetHeap();
    
private:
    TInt StartServer();
    };

#endif // MEMSPYSESSION_H
