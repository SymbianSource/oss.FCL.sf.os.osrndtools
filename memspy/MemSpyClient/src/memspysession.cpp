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


#include "memspysession.h"
#include <memspy/api/memspyapiprocess.h>
#include <memspy/api/memspyapikernelobject.h>
#include <memspy/api/memspyapikernelobjectitem.h>
//KernelObjects
#include <memspy/driver/memspydriverenumerationsshared.h>
// IMPLEMENTATION

EXPORT_C RMemSpySession::RMemSpySession()
    {
    }


EXPORT_C TInt RMemSpySession::Connect()
    {
	TInt error(KErrNone);
	
	for (TInt i=0; i<2; i++) // Two retries max
		{
		TInt error = CreateSession(KMemSpyServer2, TVersion(KMemSpyVersion, 0, 0));
		
		if (error != KErrNotFound && error != KErrServerTerminated)
			return error;
		
		error = StartServer();
		
		if (error != KErrNone && error != KErrAlreadyExists)
			return error;
		}
	
	return error;	
    }

TInt RMemSpySession::StartServer()
	{
	RProcess server;
	_LIT(KCommand, "start");
	const TUid KServerUid3 = {0xE5883BC2};
	TInt error = server.Create(KMemSpyServer2, KCommand);//, KServerUid3);
	
	if (error != KErrNone)
		return error;
	
	TRequestStatus startStatus, stopStatus;
	server.Logon(stopStatus);
	if (stopStatus != KRequestPending)
		{
		User::WaitForRequest(stopStatus);
		server.Kill(0);
		server.Close();
		return stopStatus.Int();
		}
	
	server.Rendezvous(startStatus);
	server.Resume();
	User::WaitForRequest(startStatus, stopStatus);
	if (startStatus == KRequestPending)
		{
		// not started yet, i.e. stopStatus was signaled
		server.Close();
		return stopStatus.Int();
		}
	
	// Rendezvous was called - the server is ready
	
	// We first need to cancel Logon
	server.LogonCancel(stopStatus);
	// We don't need this anymore
	server.Close();
	// Wait for LogonCancel to complete
	User::WaitForRequest(stopStatus);

	// Everything went OK
	return KErrNone;
	}

//inline void RMemSpySession::Close()
//    {
//    RSessionBase::Close();
//    }
//
//inline TMemSpySinkType RMemSpySession::GetSinkType()
//	{
//	TPckgBuf<TMemSpySinkType> OutBuf;
//	TIpcArgs args( &OutBuf );
//	SendReceive( EGetSinkType, args );
//	return OutBuf();
//	}
//
//inline void RMemSpySession::OutputKernelObjects()
//	{
//	SendReceive( EOutputKernelObjects );
//	}
//
//inline void RMemSpySession::OutputToDebug()
//	{
//	SendReceive( EOutputToDebug );
//	}
//
//inline void RMemSpySession::OutputToFile()
//	{
//	SendReceive( EOutputToFile );
//	}
//
//inline void RMemSpySession::SetServerTimeOutStatus( TUint32 aValue, TBool aEnabled )
//	{
//	TPckgBuf<TUint32> In1(aValue);
//	TPckgBuf<TBool> In2(aEnabled);
//	TIpcArgs args( &In1, &In2 );
//	SendReceive( ESetServerTimeOutStatus, args );	
//	}

//inline void RMemSpySession::OutputProcessInfo( TMemSpyProcess aProcess )
//	{
//	TProcessId iId = aProcess.iId;
//	TPckgBuf<TProcessId> In( iId );
//	TIpcArgs args( &In );
//	SendReceive( EOutputProcessInfo, args );
//	}

////Processes operations
//inline TInt RMemSpySession::ProcessesCount()
//	{
//	TPckgBuf<TInt> Out;
//	TIpcArgs args( &Out );
//	SendReceive( EProcessesCount, args );
//	return Out();
//	}
//
//inline TMemSpyProcess RMemSpySession::GetProcessByIndex( TInt aIndex )
//	{
//	TPckgBuf<TInt> In( aIndex );
//	TPckgBuf<TMemSpyProcess> Out;
//	TIpcArgs args( &In, &Out );
//	SendReceive( EProcessByIndex, args );
//	return Out();
//	}
//
//inline TInt RMemSpySession::ProcessIndexById( TProcessId aId )
//	{
//	TPckgBuf<TProcessId> In( aId );
//	TPckgBuf<TInt> Out;
//	TIpcArgs args( &In, &Out );
//	SendReceive( EProcessIndexById, args );
//	return Out();
//	}
//
//inline TBool RMemSpySession::ProcessIsDead( TMemSpyProcess aProcess )
//	{
//	TProcessId iId = aProcess.iId;
//	TPckgBuf<TProcessId> In( iId );
//	TPckgBuf<TBool> Out;
//	TIpcArgs args( &In, &Out );
//	SendReceive( EProcessIsDead, args );
//	return Out();
//	}

EXPORT_C void RMemSpySession::GetProcessesL(RArray<CMemSpyApiProcess*> &aProcesses, TSortType aSortType)
	{
	TPckgBuf<TInt> count;
	User::LeaveIfError(SendReceive(EGetProcessCount, TIpcArgs(&count)));
	
	TInt requestedCount = count();
	HBufC8* buffer = HBufC8::NewLC(requestedCount * sizeof(TMemSpyProcessData));
	TPtr8 bufferPtr(buffer->Des());
	
	User::LeaveIfError(SendReceive(EGetProcesses, TIpcArgs(&count, &bufferPtr)));
	aProcesses.Reset();
	
	TInt receivedCount = Min(count(), requestedCount);
	for(TInt i=0, offset = 0; i<requestedCount; i++, offset+=sizeof(TMemSpyProcessData))
		{
		TPckgBuf<TMemSpyProcessData> data;
		data.Copy(bufferPtr.Ptr()+offset, sizeof(TMemSpyProcessData));
		aProcesses.AppendL(CMemSpyApiProcess::NewLC(data()));
		}
	CleanupStack::Pop(aProcesses.Count());
	CleanupStack::PopAndDestroy(buffer);
	}

EXPORT_C TInt RMemSpySession::ProcessSystemPermanentOrCritical( TProcessId aId, TBool aValue )
	{
	TPckgBuf<TProcessId> arg1( aId );
	TPckgBuf<TBool> arg2( aValue );
	TIpcArgs args( &arg1, &arg2 );
		
	TInt error = SendReceive( EProcessSystemPermanentOrCritical, args );
	
	return error;
	}

EXPORT_C TInt RMemSpySession::EndProcess( TProcessId aId, TEndType aType )
	{
	TPckgBuf<TProcessId> arg1( aId );
	TPckgBuf<TEndType> arg2( aType );
	TIpcArgs args( &arg1, &arg2 );
	
	TInt error = SendReceive( EEndProcess, args );
	
	return error;
	}

EXPORT_C TInt RMemSpySession::SwitchToProcess( TProcessId aId, TBool aBrought )
	{
	TPckgBuf<TProcessId> arg1( aId );
	TPckgBuf<TBool> arg2( aBrought );
	TIpcArgs args( &arg1, &arg2 );
	
	TInt error = SendReceive( ESwitchToProcess, args );
	
	return error;	
	}

EXPORT_C void RMemSpySession::GetThreadsL(TProcessId aProcessId, RArray<CMemSpyApiThread*> &aThreads, TSortType aSortType)
	{
	TPckgBuf<TInt> count;
	TPckgBuf<TProcessId> pid(aProcessId);
	User::LeaveIfError(SendReceive(EGetThreadCount, TIpcArgs(&count, &pid)));
	
	TInt requestedCount = count();
	HBufC8* buffer = HBufC8::NewLC(requestedCount * sizeof(TMemSpyThreadData));
	TPtr8 bufferPtr(buffer->Des());
	
	User::LeaveIfError(SendReceive(EGetThreads, TIpcArgs(&count, &bufferPtr, &pid)));
	aThreads.Reset();
	
	TInt receivedCount = Min(count(), requestedCount);
	for(TInt i=0, offset = 0; i<requestedCount; i++, offset+=sizeof(TMemSpyThreadData))
		{
		TPckgBuf<TMemSpyThreadData> data;
		data.Copy(bufferPtr.Ptr()+offset, sizeof(TMemSpyThreadData));
		aThreads.AppendL(CMemSpyApiThread::NewLC(data()));
		}
	CleanupStack::Pop(aThreads.Count());
	CleanupStack::PopAndDestroy(buffer);
	}

EXPORT_C void RMemSpySession::SetThreadPriorityL(TThreadId aId, TInt aPriority)
	{
	TPckgBuf<TThreadId> arg1( aId );
	TPckgBuf<TInt> arg2( aPriority );
	
	User::LeaveIfError(SendReceive( ESetThreadPriority, TIpcArgs(&arg1, &arg2)));
	}

EXPORT_C TInt RMemSpySession::ThreadSystemPermanentOrCritical( TThreadId aId, TBool aValue )
	{
	TPckgBuf<TThreadId> arg1( aId );
	TPckgBuf<TBool> arg2( aValue );
	TIpcArgs args( &arg1, &arg2 );
	
	TInt error = SendReceive( EThreadSystemPermanentOrCritical, args );
	
	return error;
	}

EXPORT_C TInt RMemSpySession::EndThread( TThreadId aId, TEndType aType )
	{
	TPckgBuf<TThreadId> arg1( aId );
	TPckgBuf<TEndType> arg2( aType );
	TIpcArgs args( &arg1, &arg2 );
	
	TInt error = SendReceive( EEndThread, args );
	
	return error;
	}

EXPORT_C TInt RMemSpySession::SwitchToThread( TThreadId aId, TBool aBrought )
	{
	
	}
/*
EXPORT_C TInt RMemSpySession::TerminateThread( TThreadId aId )
	{
	TPckgBuf<TThreadId> arg( aId );
	TIpcArgs args( &arg );
	
	TInt error = SendReceive( ETerminateThread, args );
	
	return error;
	}
*/

//inline void RMemSpySession::SortProcessesBy( TSortType aType )
//	{
//	TPckgBuf<TSortType> In( aType );
//	TIpcArgs args( &In );
//	SendReceive( ESortProcessesBy, args );
//	}

//inline void RMemSpySession::OpenCloseCurrentProcess( TProcessId aId, TBool  aOpen )
//	{
//	TPckgBuf<TProcessId> In1( aId );
//	TPckgBuf<TBool> In2( aOpen );
//	TIpcArgs args( &In1, &In2 );
//	SendReceive( EOpenCloseCurrentProcess, args );
//	}

//Kernel Objects specific operations
EXPORT_C TInt RMemSpySession::GetKernelObjects( RArray<CMemSpyApiKernelObject*> &aKernelObjects )
	{		
	TPckgBuf<TInt> count;
	TInt error = SendReceive( EGetKernelObjectTypeCount, TIpcArgs(&count) );
	
	if( error == KErrNone )
		{			
		TInt requestedCount = count();
		HBufC8* buffer = HBufC8::NewLC(requestedCount * sizeof(TMemSpyKernelObjectData));
		TPtr8 bufferPtr(buffer->Des());
			
		TPckgBuf<TInt> count(requestedCount);
		TIpcArgs args( &count, &bufferPtr );
		TInt error = SendReceive( EGetKernelObjects, args );
			
		for(TInt i=0, offset = 0; i<requestedCount; i++, offset+=sizeof(TMemSpyKernelObjectData))
			{
			TPckgBuf<TMemSpyKernelObjectData> data;
			data.Copy(bufferPtr.Ptr()+offset, sizeof(TMemSpyKernelObjectData));
			aKernelObjects.AppendL(CMemSpyApiKernelObject::NewL(data()));
			}
				
		CleanupStack::PopAndDestroy(buffer);						
		}	
	return KErrNone;		
	}

EXPORT_C TInt RMemSpySession::GetKernelObjectItems( RArray<CMemSpyApiKernelObjectItem*> &aKernelObjectItems, TMemSpyDriverContainerType aForContainer )
	{
	TPckgBuf<TInt> count;
	TPckgBuf<TMemSpyDriverContainerType> type(aForContainer);
	TInt error = SendReceive( EGetKernelObjectItemsCount, TIpcArgs(&count, &type) );
		
	if (error == KErrNone)
		{
		TInt requestedCount = count();
		HBufC8* buffer = HBufC8::NewLC(requestedCount * sizeof(TMemSpyDriverHandleInfoGeneric));
		TPtr8 bufferPtr(buffer->Des());
		
		TPckgBuf<TInt> count(requestedCount);
		TIpcArgs args( &count, &type, &bufferPtr );
		TInt error = SendReceive( EGetKernelObjectItems, args );
		
		for(TInt i=0, offset = 0; i<requestedCount; i++, offset+=sizeof(TMemSpyDriverHandleInfoGeneric))
			{
			TPckgBuf<TMemSpyDriverHandleInfoGeneric> data;
			data.Copy(bufferPtr.Ptr()+offset, sizeof(TMemSpyDriverHandleInfoGeneric));
			aKernelObjectItems.AppendL( CMemSpyApiKernelObjectItem::NewL( data() ) );
			}
						
		CleanupStack::PopAndDestroy(buffer);				
		}
	return KErrNone;
	}

//Heap specific operations
EXPORT_C CMemSpyApiHeap* RMemSpySession::GetHeap()
	{
	CMemSpyApiHeap* aHeap;
	TInt error = KErrNone;
	
	HBufC8* buffer = HBufC8::NewLC( sizeof(TMemSpyHeapData) );
	TPtr8 bufferPtr(buffer->Des());
	TIpcArgs args( &bufferPtr );
	
	error = SendReceive( EGetHeap, args );
	
	if( error == KErrNone )
		{
		TPckgBuf<TMemSpyHeapData> data;
		data.Copy(bufferPtr.Ptr(), sizeof(TMemSpyHeapData));		
		aHeap = CMemSpyApiHeap::NewL( data() );
		}
	CleanupStack::PopAndDestroy(buffer);
		
	return aHeap;
	}
