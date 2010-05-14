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
* Implementation of the new MemSpyServer
*/

// System includes
#include <e32svr.h>
#include <e32cmn.h>
#include <e32base.h>
#include <w32std.h>
//#include <apgtask.h>
//#include <apgwgnam.h>

//user includes
#include "MemSpyServerSession.h"

// User includes
#include <memspy/engine/memspyengine.h>
#include <memspy/engine/memspyenginelogger.h>
#include <memspyengineclientinterface.h>
#include <memspy/engine/memspyengineobjectthread.h>
#include <memspy/engine/memspyengineobjectprocess.h>
#include <memspy/engine/memspyengineobjectcontainer.h>
#include <memspy/engine/memspyenginehelperchunk.h>
#include <memspy/engine/memspyenginehelpercodesegment.h>
#include <memspy/engine/memspyenginehelperheap.h>
#include <memspy/engine/memspyenginehelperstack.h>
#include <memspy/engine/memspyenginehelperthread.h>
#include <memspy/engine/memspyenginehelperprocess.h>
#include <memspy/engine/memspyenginehelperfilesystem.h>
#include <memspy/engine/memspyenginehelperram.h>
#include <memspy/engine/memspyenginehelpersysmemtracker.h>
#include <memspy/engine/memspyenginehelpersysmemtrackerconfig.h>
#include <memspy/engine/memspyenginehelperkernelcontainers.h>
#include <memspy/engine/memspyenginehelperserver.h>

//cigasto
#include <memspysession.h>
//#include <memspy/driver/memspydriverclient.h>

// ---------------------------------------------------------
// CMemSpyServerSession( CMemSpyEngine& aEngine )
// ---------------------------------------------------------
//
CMemSpyServerSession::CMemSpyServerSession( CMemSpyEngine& aEngine )
:   iEngine( aEngine )
    {	
    }

// ---------------------------------------------------------
// ~CMemSpyServerSession()
// ---------------------------------------------------------
//
CMemSpyServerSession::~CMemSpyServerSession()
    {
#ifdef _DEBUG
    TPtrC pThreadName( KNullDesC );
    if  ( iClientThreadName )
        {
        pThreadName.Set( *iClientThreadName );
        }

    RDebug::Print( _L("[MemSpy] CMemSpyServerSession::~CMemSpyServerSession() - DEAD SESSION - this: 0x%08x, id: %4d, name: %S"), this, iClientThreadId, iClientThreadName );
#endif

    delete iClientThreadName;
    }

// ---------------------------------------------------------
// ConstructL( const RMessage2& aMessage )
// ---------------------------------------------------------
//
void CMemSpyServerSession::ConstructL( const RMessage2& aMessage )
    {
	RThread thread;
    const TInt error = aMessage.Client( thread );
    CleanupClosePushL( thread );

    TRACE( RDebug::Printf( "[MemSpy] CMemSpyServerSession::ConstructL() - this: 0x%08x - opening client thread - err: %d", this, error ) );

    User::LeaveIfError( error );

    const TFullName threadName( thread.FullName() );
    iClientThreadName = threadName.AllocL();
    iClientThreadId = thread.Id();

    CleanupStack::PopAndDestroy( &thread );

    TRACE( RDebug::Print( _L("[MemSpy] CMemSpyServerSession::ConstructL() - NEW SESSION - this: 0x%08x, id: %4d, client: %S"), this, iClientThreadId, iClientThreadName ) );
    }

// ---------------------------------------------------------
// NewL( CMemSpyEngine& aEngine, const RMessage2& aMessage )
// Two-phased constructor
// ---------------------------------------------------------
//
CMemSpyServerSession* CMemSpyServerSession::NewL( CMemSpyEngine& aEngine, const RMessage2& aMessage )
    {
    CMemSpyServerSession* self = new(ELeave) CMemSpyServerSession( aEngine );
    CleanupStack::PushL( self );
    self->ConstructL( aMessage );
    CleanupStack::Pop( self );
    return self;
    }

// ---------------------------------------------------------
// ServiceL( const RMessage2& aMessage )
// Method from which the clients request arrives
// ---------------------------------------------------------
//
void CMemSpyServerSession::ServiceL( const RMessage2& aMessage )
    {
    TRACE( RDebug::Print( _L("[MemSpy] CMemSpyServerSession::ServiceL() - START - this: 0x%08x, fn: 0x%08x, id: %4d, client: %S"), this, aMessage.Function(), iClientThreadId, iClientThreadName ) );   
    
    TRAPD( error, DoServiceL( aMessage ) );
    if  ( error != KErrNone )
        {
        RDebug::Print( _L("[MemSpy] CMemSpyServerSession::ServiceL() - SERVICE ERROR - this: 0x%08x, fn: %d, err: %d, client: %S"), this, aMessage.Function(), error, iClientThreadName );
        }
    aMessage.Complete( error );

    TRACE( RDebug::Print( _L("[MemSpy] CMemSpyServerSession::ServiceL() - END - this: 0x%08x, fn: 0x%08x, id: %4d, client: %S"), this, aMessage.Function(), iClientThreadId, iClientThreadName ) );
	}

// ---------------------------------------------------------
// DoServiceL( const RMessage2& aMessage )
// ---------------------------------------------------------
//
void CMemSpyServerSession::DoServiceL( const RMessage2& aMessage )
    {
    TInt error = KErrNone;

    // Check function attributes
    const TInt function = aMessage.Function() & KMemSpyOpFlagsTypeMask;    
    const TInt argSpec = aMessage.Function() & KMemSpyOpFlagsInclusionMask;
    const TBool byThreadId = ( argSpec == KMemSpyOpFlagsIncludesThreadId );
    const TBool byThreadName = ( argSpec == KMemSpyOpFlagsIncludesThreadName );

    TRACE( RDebug::Printf( "[MemSpy] CMemSpyServerSession::DoServiceL() - START - unmodified function: 0x%08x, opCode: %d [TID: %d, TN: %d]", aMessage.Function(), function, byThreadId, byThreadName ) );   
    
    switch (function)
    	{
		case EGetProcessCount:
			{
			aMessage.WriteL(0, TPckgBuf<TInt>(iEngine.Container().Count()));
			break;
			}
		case EGetProcesses:
			{
			CMemSpyEngineObjectContainer& list = iEngine.Container();
			
			TPckgBuf<TInt> a0;
			aMessage.ReadL(0, a0);
			TInt realCount = Min(a0(), list.Count());
			
			for(TInt i=0, offset = 0; i<realCount; i++, offset += sizeof(TMemSpyProcessData))
				{
				CMemSpyProcess& process = iEngine.Container().At(i);
				TMemSpyProcessData data;
				data.iId = process.Id();
				data.iName.Copy(process.Name());
				
				TPckgBuf<TMemSpyProcessData> buffer(data);
				aMessage.WriteL(1, buffer, offset);
				}
			
			a0 = list.Count();
			aMessage.WriteL(0, a0);

			break;
			}
		case EProcessSystemPermanentOrCritical:
			{
			TBool ret = EFalse;
			TPckgBuf<TProcessId> id;
			aMessage.ReadL( 0, id );
			
			CMemSpyEngineObjectContainer& container = iEngine.Container();
			CMemSpyProcess& process = container.ProcessByIdL( id() );
			
			process.Open();

			if  ( process.IsSystemPermanent() || process.IsSystemCritical() )
				{
				ret = ETrue;
				}
			TPckgBuf<TBool> retBuf( ret );
			aMessage.WriteL( 1, retBuf );
			
			break;
			}
		case EEndProcess:
			{
			TPckgBuf<TProcessId> id;
			aMessage.ReadL( 0, id );
			TPckgBuf<TEndType> type;
			aMessage.ReadL( 1, type );
					
			CMemSpyEngineObjectContainer& container = iEngine.Container();			
			CMemSpyProcess& process = container.ProcessByIdL( id() );
									
			switch ( type() )
				{
				case ETerminate:
					{
					process.TerminateL();
					break;
					}
				case EPanic:
					{
					process.PanicL();
					break;
					}
				case EKill:
					{
					process.KillL();
					break;
					}
				}																
			break;
			}
		case ESwitchToProcess:
			{/*
		    TInt wgCount;
		    RWsSession wsSession;
		    User::LeaveIfError( wsSession.Connect() );
		    CleanupClosePushL( wsSession );
		    User::LeaveIfError( wgCount = wsSession.NumWindowGroups() );
		    RArray<RWsSession::TWindowGroupChainInfo> wgArray;
		    CleanupClosePushL( wgArray );
		    User::LeaveIfError( wsSession.WindowGroupList( &wgArray ) );
		    TApaTask task( wsSession );
		    TBool brought( EFalse );
		    TInt wgId( KErrNotFound );
		    TThreadId threadId;
			
			TPckgBuf<TProcessId> id;
			aMessage.ReadL( 0, id );
			CMemSpyEngineObjectContainer& container = iEngine.Container();
			CMemSpyProcess& process = container.ProcessByIdL( id() );
			
			// loop trough threads in a process
			for ( TInt i = 0; i < process.MdcaCount(); i++ )
				{
				TInt wgCountLocal = wgCount;
			                
				// loop trough all window groups and see if a thread id matches
				while( !brought && wgCountLocal-- )
					{
					wgId = wgArray[wgCountLocal].iId;
					User::LeaveIfError( wsSession.GetWindowGroupClientThreadId( wgId, threadId ) );
					if ( threadId == process.At( i ).Id() )
						{
						CApaWindowGroupName* wgName = CApaWindowGroupName::NewLC( wsSession, wgId );
						task.SetWgId( wgId );
						if ( !wgName->Hidden() && task.Exists() )
							{
							task.BringToForeground();
							brought = ETrue;                        
							}
						CleanupStack::PopAndDestroy( wgName );
						}
					}
				}
			
			TPckgBuf<TBool> ret( brought );
			aMessage.WriteL( 1, ret );
			
			break;*/
			}
		case EGetThreadCount:
			{
			TPckgBuf<TProcessId> pid;
			aMessage.ReadL(1, pid);
			CMemSpyProcess& process = iEngine.Container().ProcessByIdL(pid());
			process.Open();
			aMessage.WriteL(0, TPckgBuf<TInt>(process.Count()));
			process.Close();
			break;
			}
		case EGetThreads:
			{
			TPckgBuf<TProcessId> pid;
			aMessage.ReadL(2, pid);
			
			CMemSpyProcess& list = iEngine.Container().ProcessByIdL(pid());
			list.Open();
			
			TPckgBuf<TInt> a0;
			aMessage.ReadL(0, a0);
			TInt realCount = Min(a0(), list.Count());
			
			for(TInt i=0, offset = 0; i<realCount; i++, offset += sizeof(TMemSpyThreadData))
				{
				CMemSpyThread& thread = list.At(i);
				thread.Open();
				
				TMemSpyThreadData data;
				data.iId = thread.Id();
				data.iName.Copy(thread.Name());
				data.iThreadPriority = thread.Priority();
				
				TPckgBuf<TMemSpyThreadData> buffer(data);
				aMessage.WriteL(1, buffer, offset);
				
				thread.Close();
				}
			
			a0 = list.Count();
			aMessage.WriteL(0, a0);
			
			list.Close();

			break;
			}
		case ESetThreadPriority:
			{
			TPckgBuf<TThreadId> tid;
			TPckgBuf<TInt> priority;
			aMessage.ReadL(0, tid);
			aMessage.ReadL(1, priority);
			
			CMemSpyProcess* process = NULL;
			CMemSpyThread* thread = NULL; 
			User::LeaveIfError(iEngine.Container().ProcessAndThreadByThreadId(tid(), process, thread));
			
			if (thread)
				{				
				thread->Open();
				thread->SetPriorityL(static_cast<TThreadPriority>(priority()));				
				thread->Close();
				}					
			break;
			}
		case EThreadSystemPermanentOrCritical:
			{
			TBool ret = EFalse;
			TPckgBuf<TThreadId> id;
			aMessage.ReadL( 0, id );
			
			CMemSpyEngineObjectContainer& container = iEngine.Container();            
            CMemSpyProcess* process = NULL;
            CMemSpyThread* thread = NULL; 
            User::LeaveIfError( container.ProcessAndThreadByThreadId( id(), process, thread ) );
            
            if ( thread )
            	{				
				thread->Open();
				
				if  ( thread->IsSystemPermanent() || thread->IsSystemCritical() )
					{			
					ret = ETrue;					
					}
				thread->Close();
            	}
            TPckgBuf<TBool> retBuf( ret );
            aMessage.WriteL( 1, retBuf );
            				
			break;
			}
		case EEndThread:
			{
			TPckgBuf<TThreadId> id;
			aMessage.ReadL( 0, id );
			TPckgBuf<TEndType> type;
			aMessage.ReadL( 1, type );
			
			CMemSpyEngineObjectContainer& container = iEngine.Container();
			CMemSpyProcess* process = NULL;
			CMemSpyThread* thread = NULL; 
			User::LeaveIfError( container.ProcessAndThreadByThreadId( id(), process, thread ) );
			
			if( thread )
				{
				switch ( type() )
					{
					case ETerminate:
						{
						thread->TerminateL();
						break;
						}
					case EPanic:
						{
						thread->PanicL();
						break;
						}
					case EKill:
						{
						thread->KillL();
						break;
						}
					}				
				}			
			break;
			}
		// --- KernelObjects related functions ---
		case EGetKernelObjectTypeCount:
			{
			TInt iCount = EMemSpyDriverContainerTypeChunk - EMemSpyDriverContainerTypeFirst;
			//TInt iCount = EMemSpyDriverContainerTypeLast - EMemSpyDriverContainerTypeFirst;
			TPckgBuf<TInt> ret( iCount );
			aMessage.WriteL(0, ret);			
			break;
			}
		case EGetKernelObjects:
			{
			TPckgBuf<TInt> count;
			aMessage.ReadL(0, count);
			
			CMemSpyEngineGenericKernelObjectContainer* model = iEngine.HelperKernelContainers().ObjectsAllL(); //contains all the objects
			CleanupStack::PushL( model );
			
			for( TInt i=0, offset = 0; i<count(); i++, offset += sizeof( TMemSpyKernelObjectData ) )
				{
				TMemSpyKernelObjectData data;								
												
				data.iName.Append( model->At(i).Name() );
				data.iType = model->At(i).Type();
				data.iCount = model->At(i).Count();											
				data.iSize = model->At(i).Count() * model->At(i).Count();

				TPckgBuf<TMemSpyKernelObjectData> buffer(data);
				aMessage.WriteL(1, buffer, offset);
				}			
			aMessage.WriteL(0, count);
			CleanupStack::PopAndDestroy( model );
			break;
			}
		case EGetKernelObjectItemsCount:
			{
			TPckgBuf<TMemSpyDriverContainerType> tempType;
			aMessage.ReadL(1, tempType); //get type of kernel object
			TMemSpyDriverContainerType type = tempType();
			
			CMemSpyEngineHelperKernelContainers& kernelContainerManager = iEngine.HelperKernelContainers();
			CMemSpyEngineGenericKernelObjectList* iObjectList = kernelContainerManager.ObjectsForSpecificContainerL( type );
			CleanupStack::PushL( iObjectList );
			
			TInt count = iObjectList->Count();
			TPckgBuf<TInt> ret( count );
			aMessage.WriteL( 0, ret );
			
			CleanupStack::PopAndDestroy( iObjectList );
			break;
			}
		case EGetKernelObjectItems:
			{
			TPckgBuf<TInt> count;
			TPckgBuf<TMemSpyDriverContainerType> tempType;
			aMessage.ReadL( 0, count ); //get count of items
			aMessage.ReadL(1, tempType); //get type of kernel object
			TInt c = count();
			
			CMemSpyEngineHelperKernelContainers& kernelContainerManager = iEngine.HelperKernelContainers();
			CMemSpyEngineGenericKernelObjectList* iObjectList = kernelContainerManager.ObjectsForSpecificContainerL( tempType() );
			CleanupStack::PushL( iObjectList );
			
			for( TInt i=0, offset = 0; i<c; i++, offset += sizeof( TMemSpyDriverHandleInfoGeneric ) )
				{
				TMemSpyDriverHandleInfoGeneric data;								
															
				data = iObjectList->At( i );
				
				TPckgBuf<TMemSpyDriverHandleInfoGeneric> buffer(data);
				aMessage.WriteL(2, buffer, offset);
				}			
			
			CleanupStack::PopAndDestroy( iObjectList );			
			break;
			}
		// --- Kernel Heap related functions ---
		case EGetHeap:
			{
			TMemSpyHeapInfo heapInfo;			
			iEngine.HelperHeap().GetHeapInfoKernelL( heapInfo );
			TMemSpyHeapData data = iEngine.HelperHeap().NewHeapRawInfo( heapInfo );
			
			TPckgBuf<TMemSpyHeapData> buffer(data);
			aMessage.WriteL(0, buffer);
			
			break;
			}
    	}
//    else if( function == EProcessByIndex )
//    	{
//		TMemSpyProcess iProcess;
//		TInt in;
//		TPckgBuf<TInt> index( in );
//		aMessage.ReadL( 0, index );
//		CMemSpyProcess& process = iEngine.Container().At( index() );
//		//fill the data structure
//		iProcess.iId = process.Id(); //process ID
//		iProcess.iName.Append( process.Name() ); //raw process name
//		iProcess.iThreadCount = process.Count(); //thread count
//		iProcess.iPriority = process.Priority();
//		//			
//		TPckgBuf<TMemSpyProcess> buf( iProcess );
//		aMessage.WriteL( 1, buf );
//		aMessage.Complete( KErrNone );
//    	}
//    else if( function == EProcessIndexById )
//    	{
//		TProcessId iId;
//		TPckgBuf<TProcessId> buf( iId );
//    	aMessage.ReadL( 0, buf );
//    	TInt index = iEngine.Container().ProcessIndexById( buf() );
//    	TPckgBuf<TInt> out( index );
//    	aMessage.WriteL( 1, out );
//    	aMessage.Complete( KErrNone );
//    	}
//    else if( function == EOutputProcessInfo )
//    	{
//		TProcessId iId;
//		TPckgBuf<TProcessId> buf( iId );
//		aMessage.ReadL( 0, buf );
//		CMemSpyProcess& process = iEngine.Container().ProcessByIdL( buf() );
//		iEngine.HelperProcess().OutputProcessInfoL( process );
//    	}
//    else if( function == EProcessIsDead )
//    	{
//		TProcessId iId;
//		TPckgBuf<TProcessId> buf( iId );
//		aMessage.ReadL( 0, buf );
//		CMemSpyProcess& process = iEngine.Container().ProcessByIdL( buf() );
//		TBool dead = process.IsDead();
//		TPckgBuf<TBool> out( dead );
//		aMessage.WriteL( 1, out );
//		aMessage.Complete( KErrNone );
//    	}
//    else if( function == ESortProcessesBy )
//    	{
//		TSortType type;
//		TPckgBuf<TSortType> buf(type);
//		aMessage.ReadL( 0, buf );
//		if( buf() == ESortProcById )
//			{
//			iEngine.Container().SortById();
//			}
//		else if( buf() == ESortProcByName )
//			{
//			iEngine.Container().SortByName();
//			}
//		else if( buf() == ESortProcByThreadCount )
//			{
//			iEngine.Container().SortByThreadCount();
//			}
//		else if( buf() == ESortProcByCodeSegs )
//			{
//			iEngine.Container().SortByCodeSegs();
//			}
//		else if( buf() == ESortProcByHeapUsage )
//			{
//			iEngine.Container().SortByHeapUsage();			
//			}
//		else if( buf() == ESortProcByStackUsage )
//			{
//			iEngine.Container().SortByStackUsage();
//			}			
//    	}
//    else if( function == EOpenCloseCurrentProcess )
//    	{
//		TProcessId id;
//		TBool open;
//    	TPckgBuf<TProcessId> buf(id);
//    	TPckgBuf<TBool> buf2(open);
//    	aMessage.ReadL( 0, buf );
//    	aMessage.ReadL( 1, buf2 );
//    	if( buf2() == 1 )
//    		{
//			iEngine.Container().ProcessByIdL( buf() ).Open();
//    		}
//    	else
//    		{
//			iEngine.Container().ProcessByIdL( buf() ).Close();
//    		}
//    	}
    
//    // Check function is supported and argument combination is valid
//    error = ValidateFunction( function, byThreadId, byThreadName );
//    TRACE( RDebug::Printf( "[MemSpy] CMemSpyServerSession::DoServiceL() - validation result: %d", error ) );
//    
//    // Process function request
//    if  ( error == KErrNone )
//        {
//        if  ( byThreadId )
//            {
//            TRACE( RDebug::Printf( "[MemSpy] CMemSpyServerSession::DoServiceL() - [TID] thread-specific..." ) );
//            
//            const TThreadId threadId( aMessage.Int0() );
//            HandleThreadSpecificOpL( function, threadId );
//            }
//        else if ( byThreadName )
//            {
//            TRACE( RDebug::Printf( "[MemSpy] CMemSpyServerSession::DoServiceL() - [TN] thread-specific..." ) );
//
//            error = aMessage.GetDesLength( 0 /*slot 0*/ );
//        
//            if  ( error > 0 && error <= KMaxFullName )
//                {
//                TFullName* threadName = new(ELeave) TFullName();
//                CleanupStack::PushL( threadName );
//                aMessage.ReadL( 0, *threadName );
//                HandleThreadSpecificOpL( function, *threadName );
//                CleanupStack::PopAndDestroy( threadName );
//                }
//            else
//                {
//                error = KErrArgument;
//                }
//            }
//        else
//            {
//            TRACE( RDebug::Printf( "[MemSpy] CMemSpyServerSession::DoServiceL() - thread-agnostic..." ) );
//
//            HandleThreadAgnosticOpL( function, aMessage );
//            }
//        }

    User::LeaveIfError( error );

    TRACE( RDebug::Printf( "[MemSpy] CMemSpyServerSession::DoServiceL() - HELLOOOOOO" ) );
    TRACE( RDebug::Printf( "[MemSpy] CMemSpyServerSession::DoServiceL() - END" ) );
    }

// ---------------------------------------------------------
// ValidateFunction( TInt aFunction, TBool aIncludesThreadId, TBool aIncludesThreadName )
// Validates the MemSpy operation types
// ---------------------------------------------------------
//
TInt CMemSpyServerSession::ValidateFunction( TInt aFunction, TBool aIncludesThreadId, TBool aIncludesThreadName )
    {
    TInt err = KErrNotSupported;
    
    // Check the operation is within op-code range
    if  ( aFunction >= EMemSpyClientServerOpMarkerFirst && aFunction < EMemSpyClientServerOpMarkerLast )
        {
        // Check the operation doesn't include unnecessary or not supported information
        const TBool includesThreadIdentifier = ( aIncludesThreadId || aIncludesThreadName );
        if  ( includesThreadIdentifier && aFunction >= EMemSpyClientServerOpMarkerThreadAgnosticFirst )
            {
            // Passing a thread identifier to a thread agnostic operation
            err = KErrArgument;
            }
        else
            {
            err = KErrNone;
            }
        }
    //
    if  ( err != KErrNone )
        {
        RDebug::Printf( "[MemSpy] CMemSpyServerSession::ValidateFunction() - function request did not validate - [withId: %d, withName: %d]", aIncludesThreadId, aIncludesThreadName );
        }
    //
    return err;
    }

// ---------------------------------------------------------
// HandleThreadSpecificOpL( TInt aFunction, const TThreadId& aThreadId )
// ---------------------------------------------------------
//
void CMemSpyServerSession::HandleThreadSpecificOpL( TInt aFunction, const TThreadId& aThreadId )
    {
//    TRACE( RDebug::Printf( "[MemSpy] CMemSpyServerSession::HandleThreadSpecificOpL() - START - aFunction: %d, aThreadId: %d", aFunction, (TUint) aThreadId ) );
//
//    ASSERT( (TUint) aThreadId != 0 );
//    TInt error = KErrNone;
//
//    // Check if its a kernel thread identifier
//    const TBool isKernel = ( static_cast<TUint32>( aThreadId ) == KMemSpyClientServerThreadIdKernel );
//
//    // Treat as thread specific operation
//    CMemSpyProcess* process = NULL;
//    CMemSpyThread* thread = NULL;
//    if  ( !isKernel )
//        {
//        error = iEngine.Container().ProcessAndThreadByThreadId( aThreadId, process, thread );
//        TRACE( RDebug::Printf( "[MemSpy] CMemSpyServerSession::HandleThreadSpecificOpL() - search result: %d, proc: 0x%08x, thread: 0x%08x", error, process, thread ) );
//        }
//    else
//        {
//        // Kernel is only supported for a couple of operations
//        if  ( aFunction == EMemSpyClientServerOpHeapInfo || aFunction == EMemSpyClientServerOpHeapData )
//            {
//            }
//        else
//            {
//            TRACE( RDebug::Printf( "[MemSpy] CMemSpyServerSession::HandleThreadSpecificOpL() - trying to call unsupported function for kernel thread!" ) );
//            error = KErrArgument;
//            }
//        }
//
//    // Must be no error so far and we must have a valid thread & process when performing a non-kernel op
//    // or then if we are performing a kernel op, we don't need the thread or process.
//    if  ( error == KErrNone && ( ( thread && process && !isKernel ) || ( isKernel ) ) )
//        {
//#ifdef _DEBUG
//        if  ( thread )
//            {
//            HBufC* threadName = thread->FullName().AllocLC();
//            _LIT( KTrace2, "[MemSpy] CMemSpyServerSession::HandleThreadSpecificOpL() - thread: %S" );
//            RDebug::Print( KTrace2, threadName );
//            CleanupStack::PopAndDestroy( threadName );
//            }
//        else if ( isKernel )
//            {
//            _LIT( KTrace2, "[MemSpy] CMemSpyServerSession::HandleThreadSpecificOpL() - thread: Kernel" );
//            RDebug::Print( KTrace2 );
//            }
//#endif
//
//        // Got a valid thread object - now work out which operation to perform...
//        switch( aFunction )
//            {
//        case EMemSpyClientServerOpSummaryInfo:
//            iEngine.HelperProcess().OutputProcessInfoL( *process );
//            break;
//        case EMemSpyClientServerOpSummaryInfoDetailed:
//            iEngine.HelperProcess().OutputProcessInfoDetailedL( *process );
//            break;
//        case EMemSpyClientServerOpHeapInfo:
//            if  ( isKernel )
//                {
//                iEngine.HelperHeap().OutputHeapInfoKernelL();
//                }
//            else
//                {
//                iEngine.HelperHeap().OutputHeapInfoUserL( *thread );
//                }
//            break;
//        case EMemSpyClientServerOpHeapCellListing:
//            iEngine.HelperHeap().OutputCellListingUserL( *thread );
//            break;
//        case EMemSpyClientServerOpHeapData:
//            if  ( isKernel )
//                {
//                iEngine.HelperHeap().OutputHeapDataKernelL();
//                }
//            else
//                {
//                iEngine.HelperHeap().OutputHeapDataUserL( *thread );
//                }
//            break;
//        case EMemSpyClientServerOpStackInfo:
//            iEngine.HelperStack().OutputStackInfoL( *thread );
//            break;
//        case EMemSpyClientServerOpStackDataUser:
//            iEngine.HelperStack().OutputStackDataL( *thread, EMemSpyDriverDomainUser, EFalse );
//            break;
//        case EMemSpyClientServerOpStackDataKernel:
//            iEngine.HelperStack().OutputStackDataL( *thread, EMemSpyDriverDomainKernel, EFalse );
//            break;
//        case EMemSpyClientServerOpOpenFiles:
//            iEngine.HelperFileSystem().ListOpenFilesL( aThreadId );
//            break;
//
//        default:
//            error = KErrNotSupported;
//            break;
//            }
//        }
//
//    TRACE( RDebug::Printf("[MemSpy] CMemSpyServerSession::HandleThreadSpecificOpL() - END - aFunction: %d, aThreadId: %d, error: %d", aFunction, (TUint) aThreadId, error ) );
//    User::LeaveIfError( error );
    }

// ---------------------------------------------------------
// HandleThreadSpecificOpL( TInt aFunction, const TDesC& aThreadName )
// Gets thread ID from aThreadName 
// and calls HandleThreadSpecificOpL( , const TThreadId& aThreadId )
// ---------------------------------------------------------
//
void CMemSpyServerSession::HandleThreadSpecificOpL( TInt aFunction, const TDesC& aThreadName )
    {
//    TRACE( RDebug::Print( _L("[MemSpy] CMemSpyServerSession::HandleThreadSpecificOpL() - START - aFunction: %d, aThreadName: %S"), aFunction, &aThreadName ) );
//    //
//    CMemSpyProcess* process = NULL;
//    CMemSpyThread* thread = NULL;
//    TInt error = iEngine.Container().ProcessAndThreadByPartialName( aThreadName, process, thread );
//    User::LeaveIfError( error );
//    //
//    const TThreadId threadId( thread->Id() );
//    HandleThreadSpecificOpL( aFunction, threadId );
//    //
//    TRACE( RDebug::Print( _L("[MemSpy] CMemSpyServerSession::HandleThreadSpecificOpL() - END - aFunction: %d, aThreadName: %S"), aFunction, &aThreadName ) );
    }

// ---------------------------------------------------------
// HandleThreadAgnosticOpL( TInt aFunction, const RMessage2& aMessage )
// ---------------------------------------------------------
//
void CMemSpyServerSession::HandleThreadAgnosticOpL( TInt aFunction, const RMessage2& aMessage )
    {
//    TRACE( RDebug::Printf("[MemSpy] CMemSpyServerSession::HandleThreadAgnosticOpL() - START" ) );
//    //
//    if  ( aFunction ==  EMemSpyClientServerOpHeapInfoCompact )
//        {
//        TRACE( RDebug::Printf("[MemSpy] CMemSpyServerSession::HandleThreadAgnosticOpL() - EMemSpyClientServerOpHeapInfoCompact") );
//        iEngine.HelperHeap().OutputHeapInfoForDeviceL();
//        }
//    else if ( aFunction ==  EMemSpyClientServerOpStackInfoCompact )
//        {
//        TRACE( RDebug::Printf("[MemSpy] CMemSpyServerSession::HandleThreadAgnosticOpL() - EMemSpyClientServerOpStackInfoCompact") );
//        iEngine.HelperStack().OutputStackInfoForDeviceL();
//        }
//    else if ( aFunction == EMemSpyClientServerOpSystemWideMemoryTrackingTimerStart )
//        {
//        TRACE( RDebug::Printf("[MemSpy] CMemSpyServerSession::HandleThreadAgnosticOpL() - EMemSpyClientServerOpSystemWideMemoryTrackingTimerStart") );
//        iEngine.HelperSysMemTracker().StartL();
//        }
//    else if ( aFunction == EMemSpyClientServerOpSystemWideMemoryTrackingTimerStop )
//        {
//        TRACE( RDebug::Printf("[MemSpy] CMemSpyServerSession::HandleThreadAgnosticOpL() - EMemSpyClientServerOpSystemWideMemoryTrackingTimerStop") );
//        iEngine.HelperSysMemTracker().StopL();
//        }
//    else if ( aFunction == EMemSpyClientServerOpSystemWideMemoryTrackingReset )
//        {
//        TRACE( RDebug::Printf("[MemSpy] CMemSpyServerSession::HandleThreadAgnosticOpL() - EMemSpyClientServerOpSystemWideMemoryTrackingReset") );
//        iEngine.HelperSysMemTracker().Reset();
//        }
//    else if ( aFunction == EMemSpyClientServerOpSystemWideMemoryTrackingForceUpdate )
//        {
//        TRACE( RDebug::Printf("[MemSpy] CMemSpyServerSession::HandleThreadAgnosticOpL() - EMemSpyClientServerOpSystemWideMemoryTrackingForceUpdate") );
//        iEngine.HelperSysMemTracker().CheckForChangesNowL();
//        }
//    else if ( aFunction == EMemSpyClientServerOpSystemWideMemoryTrackingTimerPeriodSet )
//        {
//        TRACE( RDebug::Printf("[MemSpy] CMemSpyServerSession::HandleThreadAgnosticOpL() - EMemSpyClientServerOpSystemWideMemoryTrackingTimerPeriodSet") );
//        
//        // Get current config
//        TMemSpyEngineHelperSysMemTrackerConfig config;
//        iEngine.HelperSysMemTracker().GetConfig( config );
//
//        // Set new timer value
//        config.iTimerPeriod = aMessage.Int0();
//
//        // And update config... which will leave if the config is invalid
//        iEngine.HelperSysMemTracker().SetConfigL( config );
//        }
//    else if ( aFunction == EMemSpyClientServerOpSystemWideMemoryTrackingCategoriesSet )
//        {
//        TRACE( RDebug::Printf("[MemSpy] CMemSpyServerSession::HandleThreadAgnosticOpL() - EMemSpyClientServerOpSystemWideMemoryTrackingCategoriesSet") );
//        // Get current config
//        TMemSpyEngineHelperSysMemTrackerConfig config;
//        iEngine.HelperSysMemTracker().GetConfig( config );
//
//        // Set new categories
//        config.iEnabledCategories = aMessage.Int0();
//
//        // And update config... which will leave if the config is invalid
//        iEngine.HelperSysMemTracker().SetConfigL( config );
//        }
//    else if ( aFunction == EMemSpyClientServerOpSystemWideMemoryTrackingThreadNameFilterSet )
//        {
//        TRACE( RDebug::Printf("[MemSpy] CMemSpyServerSession::HandleThreadAgnosticOpL() - EMemSpyClientServerOpSystemWideMemoryTrackingThreadNameFilterSet") );
//        // Get current config
//        TMemSpyEngineHelperSysMemTrackerConfig config;
//        iEngine.HelperSysMemTracker().GetConfig( config );
//
//        // Set new filter
//        RBuf buf;
//        buf.CleanupClosePushL();
//        TInt len = aMessage.GetDesLength( 0 );
//        if ( len > 0 )
//            {
//            buf.CreateL( len );
//            aMessage.ReadL( 0, buf, 0 );
//            config.iThreadNameFilter.Copy( buf );            
//            }
//        else
//            {
//            config.iThreadNameFilter.Zero();
//            }
//        CleanupStack::PopAndDestroy( &buf );
//
//        // And update config... which will leave if the config is invalid
//        iEngine.HelperSysMemTracker().SetConfigL( config );
//        }
//    else if ( aFunction == EMemSpyClientServerOpSystemWideMemoryTrackingHeapDumpSet )
//        {
//        // Get current config
//        TMemSpyEngineHelperSysMemTrackerConfig config;
//        iEngine.HelperSysMemTracker().GetConfig( config );
//        
//        // Set new Heap Dump value
//        config.iDumpData = aMessage.Int0();
//        
//        // And update config... which will leave if the config is invalid
//        iEngine.HelperSysMemTracker().SetConfigL( config );
//        }
//    else if ( aFunction == EMemSpyClientServerOpSwitchOutputSinkTrace )
//        {
//        TRACE( RDebug::Printf("[MemSpy] CMemSpyServerSession::HandleThreadAgnosticOpL() - EMemSpyClientServerOpSwitchOutputSinkTrace") );
//        iEngine.InstallSinkL( ESinkTypeDebug );
//        }
//    else if ( aFunction == EMemSpyClientServerOpSwitchOutputSinkFile )
//        {
//        TRACE( RDebug::Printf("[MemSpy] CMemSpyServerSession::HandleThreadAgnosticOpL() - EMemSpyClientServerOpSwitchOutputSinkFile") );
//        iEngine.InstallSinkL( ESinkTypeFile );
//        }
//    else if ( aFunction == EMemSpyClientServerOpEnumerateKernelContainer )
//        {
//        const TMemSpyDriverContainerType type = CMemSpyEngineHelperKernelContainers::MapToType( static_cast< TObjectType >( aMessage.Int0() ) );
//
//        TRACE( RDebug::Printf("[MemSpy] CMemSpyServerSession::HandleThreadAgnosticOpL() - EMemSpyClientServerOpEnumerateKernelContainer - type: %d", type ) );
//
//        CMemSpyEngineGenericKernelObjectList* model = iEngine.HelperKernelContainers().ObjectsForSpecificContainerL( type );
//        CleanupStack::PushL( model );
//        model->OutputL( iEngine.Sink() );
//        CleanupStack::PopAndDestroy( model );
//        }
//    else if ( aFunction == EMemSpyClientServerOpEnumerateKernelContainerAll )
//        {
//        TRACE( RDebug::Printf("[MemSpy] CMemSpyServerSession::HandleThreadAgnosticOpL() - EMemSpyClientServerOpEnumerateKernelContainerAll") );
//        CMemSpyEngineGenericKernelObjectContainer* model = iEngine.HelperKernelContainers().ObjectsAllL();
//        CleanupStack::PushL( model );
//        model->OutputL( iEngine.Sink() );
//        CleanupStack::PopAndDestroy( model );
//        }
//    else if ( aFunction == EMemSpyClientServerOpOpenFiles )
//        {
//        TRACE( RDebug::Printf("[MemSpy] CMemSpyServerSession::HandleThreadAgnosticOpL() - EMemSpyClientServerOpOpenFiles") );
//        iEngine.ListOpenFilesL();
//        }
//    else if ( aFunction == EMemSpyClientServerOpDisableAknIconCache )
//        {
//        TRACE( RDebug::Printf("[MemSpy] CMemSpyServerSession::HandleThreadAgnosticOpL() - EMemSpyClientServerOpDisableAknIconCache") );
//        iEngine.HelperRAM().SetAknIconCacheStatusL( EFalse );
//        }
//    else
//        {
//        TRACE( RDebug::Printf("[MemSpy] CMemSpyServerSession::HandleThreadAgnosticOpL() - [device-wide operation] => invoking UI") );
//        iEngine.NotifyClientServerOperationRequestL( aFunction );
//        }
//    //
//    TRACE( RDebug::Printf("[MemSpy] CMemSpyServerSession::HandleThreadAgnosticOpL() - END" ) );
    }
