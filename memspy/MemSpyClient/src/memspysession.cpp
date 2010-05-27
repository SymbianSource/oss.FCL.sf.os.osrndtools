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
#include <memspyengineclientinterface.h>

#include <memspy/api/memspyapiprocess.h>
#include <memspy/api/memspyapikernelobject.h>
#include <memspy/api/memspyapikernelobjectitem.h>
#include <memspy/api/memspyapithreadinfoitem.h>
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
		TInt error = CreateSession(KMemSpyServerName, TVersion(KMemSpyVersion, 0, 0));
		
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
	TInt error = server.Create(KMemSpyServerName, KCommand);//, KServerUid3);
	
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
	User::LeaveIfError(SendReceive(EMemSpyClientServerOpGetProcessCount, TIpcArgs(&count)));
	
	TInt requestedCount = count();
	HBufC8* buffer = HBufC8::NewLC(requestedCount * sizeof(TMemSpyProcessData));
	TPtr8 bufferPtr(buffer->Des());
	
	User::LeaveIfError(SendReceive(EMemSpyClientServerOpGetProcesses, TIpcArgs(&count, &bufferPtr)));
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

EXPORT_C TProcessId RMemSpySession::GetProcessIdByNameL(const TDesC& aProcessName)
	{
	TPckgBuf<TProcessId> procId;
	User::LeaveIfError(SendReceive(EMemSpyClienServerOpGetProcessIdByName, 
			TIpcArgs(&aProcessName, &procId)));
	
	return procId();
	}

EXPORT_C TInt RMemSpySession::ProcessSystemPermanentOrCritical( TProcessId aId, TBool aValue )
	{
	TPckgBuf<TProcessId> arg1( aId );
	TPckgBuf<TBool> arg2( aValue );
	TIpcArgs args( &arg1, &arg2 );
		
	TInt error = SendReceive( EMemSpyClientServerOpProcessSystemPermanentOrCritical, args );
	
	return error;
	}

EXPORT_C TInt RMemSpySession::EndProcess( TProcessId aId, TMemSpyEndType aType )
	{
	TPckgBuf<TProcessId> arg1( aId );
	TPckgBuf<TMemSpyEndType> arg2( aType );
	TIpcArgs args( &arg1, &arg2 );
	
	TInt error = SendReceive( EMemSpyClientServerOpEndProcess, args );
	
	return error;
	}

EXPORT_C TInt RMemSpySession::SwitchToProcess( TProcessId aId, TBool aBrought )
	{
	TPckgBuf<TProcessId> arg1( aId );
	TPckgBuf<TBool> arg2( aBrought );
	TIpcArgs args( &arg1, &arg2 );
	
	TInt error = SendReceive( EMemSpyClientServerOpSwitchToProcess, args );
	
	return error;	
	}

EXPORT_C void RMemSpySession::GetThreadsL(TProcessId aProcessId, RArray<CMemSpyApiThread*> &aThreads, TSortType aSortType)
	{
	TPckgBuf<TInt> count;
	TPckgBuf<TProcessId> pid(aProcessId);
	User::LeaveIfError(SendReceive(EMemSpyClientServerOpGetThreadCount, TIpcArgs(&count, &pid)));
	
	TInt requestedCount = count();
	HBufC8* buffer = HBufC8::NewLC(requestedCount * sizeof(TMemSpyThreadData));
	TPtr8 bufferPtr(buffer->Des());
	
	User::LeaveIfError(SendReceive(EMemSpyClientServerOpGetThreads, TIpcArgs(&count, &bufferPtr, &pid)));
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
	
	User::LeaveIfError(SendReceive( EMemSpyClientServerOpSetThreadPriority, TIpcArgs(&arg1, &arg2)));
	}

EXPORT_C TInt RMemSpySession::ThreadSystemPermanentOrCritical( TThreadId aId, TBool aValue )
	{
	TPckgBuf<TThreadId> arg1( aId );
	TPckgBuf<TBool> arg2( aValue );
	TIpcArgs args( &arg1, &arg2 );
	
	TInt error = SendReceive( EMemSpyClientServerOpThreadSystemPermanentOrCritical, args );
	
	return error;
	}

EXPORT_C TInt RMemSpySession::EndThread( TThreadId aId, TMemSpyEndType aType )
	{
	TPckgBuf<TThreadId> arg1( aId );
	TPckgBuf<TMemSpyEndType> arg2( aType );
	TIpcArgs args( &arg1, &arg2 );
	
	TInt error = SendReceive( EMemSpyClientServerOpEndThread, args );
	
	return error;
	}

EXPORT_C TInt RMemSpySession::SwitchToThread( TThreadId aId, TBool aBrought )
	{
	TPckgBuf<TThreadId> arg1( aId );
	TPckgBuf<TBool> arg2( aBrought );
	TIpcArgs args( &arg1, &arg2 );
	
	TInt error = SendReceive( EMemSpyClientServerOpSwitchToThread, args );
	
	return error;	
	}

EXPORT_C TInt RMemSpySession::GetInfoItemType( TInt aIndex, TThreadId aId, TMemSpyThreadInfoItemType &aType )
	{
	TPckgBuf<TInt> arg1( aIndex );
	TPckgBuf<TThreadId> arg2( aId );
	TPckgBuf<TMemSpyThreadInfoItemType> arg3;
	TIpcArgs args( &arg1, &arg2, &arg3 );
		
	TInt error = SendReceive( EMemSpyClientServerOpGetInfoItemType, args );
	
	aType = arg3();
	
	return error;
	}

EXPORT_C void RMemSpySession::GetThreadInfoItems( RArray<CMemSpyApiThreadInfoItem*> &aInfoItems, TThreadId aId, TMemSpyThreadInfoItemType aType )
	{
	TPckgBuf<TThreadId> id( aId );	
	TPckgBuf<TMemSpyThreadInfoItemType> type( aType );
	TPckgBuf<TInt> count;	
	
	TInt error = SendReceive( EMemSpyClientServerOpGetThreadInfoItemsCount, TIpcArgs( &id, &type, &count ) );
	TInt itemCount = count();
	
	if( error == KErrNone )
		{		
		if( itemCount == 0 )
			{
			aInfoItems.Reset();
			}
		else
			{
			HBufC8* buffer = HBufC8::NewLC( itemCount * sizeof(TMemSpyThreadInfoItemData) );
			TPtr8 bufferPtr(buffer->Des());
			
			TPckgBuf<TInt> requestedCount( itemCount );
			
			TIpcArgs args( &requestedCount, &id, &type, &bufferPtr );
			TInt error = SendReceive( EMemSpyClientServerOpGetThreadInfoItems, args );
		
			for(TInt i=0, offset = 0; i < itemCount; i++, offset+=sizeof(TMemSpyThreadInfoItemData))
				{
				TPckgBuf<TMemSpyThreadInfoItemData> data;
				data.Copy(bufferPtr.Ptr()+offset, sizeof(TMemSpyThreadInfoItemData));
				aInfoItems.AppendL(CMemSpyApiThreadInfoItem::NewL(data()));
				}
						
			CleanupStack::PopAndDestroy(buffer);
			}
		}		
	}

//Kernel Objects specific operations
EXPORT_C TInt RMemSpySession::GetKernelObjects( RArray<CMemSpyApiKernelObject*> &aKernelObjects )
	{		
	TPckgBuf<TInt> count;
	TInt error = SendReceive( EMemSpyClientServerOpGetKernelObjectCount, TIpcArgs(&count) );
	
	if( error == KErrNone )
		{			
		TInt requestedCount = count();
		HBufC8* buffer = HBufC8::NewLC(requestedCount * sizeof(TMemSpyKernelObjectData));
		TPtr8 bufferPtr(buffer->Des());
			
		TPckgBuf<TInt> count(requestedCount);
		TIpcArgs args( &count, &bufferPtr );
		TInt error = SendReceive( EMemSpyClientServerOpGetKernelObjects, args );
			
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
	TInt error = SendReceive( EMemSpyClientServerOpGetKernelObjectItemCount, TIpcArgs(&count, &type) );
		
	if (error == KErrNone)
		{
		TInt requestedCount = count();
		HBufC8* buffer = HBufC8::NewLC(requestedCount * sizeof(TMemSpyDriverHandleInfoGeneric));
		TPtr8 bufferPtr(buffer->Des());
		
		TPckgBuf<TInt> count(requestedCount);
		TIpcArgs args( &count, &type, &bufferPtr );
		TInt error = SendReceive( EMemSpyClientServerOpGetKernelObjectItems, args );
		
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

EXPORT_C void RMemSpySession::OutputAllContainerContents()
	{
	SendReceive( EMemSpyClientServerOpOutputAllContainerContents );
	}


//Heap specific operations
EXPORT_C CMemSpyApiHeap* RMemSpySession::GetHeap()
	{
	CMemSpyApiHeap* aHeap;
	TInt error = KErrNone;
	
	HBufC8* buffer = HBufC8::NewLC( sizeof(TMemSpyHeapData) );
	TPtr8 bufferPtr(buffer->Des());
	TIpcArgs args( &bufferPtr );
	
	error = SendReceive( EMemSpyClientServerOpGetHeap, args );
	
	if( error == KErrNone )
		{
		TPckgBuf<TMemSpyHeapData> data;
		data.Copy(bufferPtr.Ptr(), sizeof(TMemSpyHeapData));		
		aHeap = CMemSpyApiHeap::NewL( data() );
		}
	CleanupStack::PopAndDestroy(buffer);
		
	return aHeap;
	}

EXPORT_C void RMemSpySession::DumpKernelHeap()
	{
	SendReceive( EMemSpyClientServerOpDumpKernelHeap );
	}

EXPORT_C void RMemSpySession::OutputKernelHeapDataL(TMemSpyOutputType aOutputType)
	{
	SetOutputTypeL(aOutputType);
	
	User::LeaveIfError(SendReceive(EMemSpyClientServerOpHeapData | KMemSpyOpFlagsIncludesThreadId,
			TIpcArgs(KMemSpyClientServerThreadIdKernel)));
	
	}
EXPORT_C void RMemSpySession::OutputThreadHeapDataL(TMemSpyOutputType aOutputType, TThreadId aThreadId)
	{
	SetOutputTypeL(aOutputType);
		
	User::LeaveIfError(SendReceive(EMemSpyClientServerOpHeapData | KMemSpyOpFlagsIncludesThreadId,
			TIpcArgs(aThreadId)));
	}
EXPORT_C void RMemSpySession::OutputThreadCellListL(TMemSpyOutputType aOutputType, TThreadId aThreadId)
	{
	SetOutputTypeL(aOutputType);
			
	User::LeaveIfError(SendReceive(EMemSpyClientServerOpHeapCellListing | KMemSpyOpFlagsIncludesThreadId,
			TIpcArgs(aThreadId)));
	}

EXPORT_C void RMemSpySession::OutputKernelObjectsL(TMemSpyOutputType aOutputType)
	{
	SetOutputTypeL(aOutputType);
				
	User::LeaveIfError(SendReceive(EMemSpyClientServerOpEnumerateKernelContainerAll));
	}

EXPORT_C void RMemSpySession::OutputCompactStackInfoL(TMemSpyOutputType aOutputType)
	{
	SetOutputTypeL(aOutputType);
					
	User::LeaveIfError(SendReceive(EMemSpyClientServerOpStackInfoCompact)); 
	}

EXPORT_C void RMemSpySession::OutputCompactHeapInfoL(TMemSpyOutputType aOutputType)
	{
	SetOutputTypeL(aOutputType);
						
	User::LeaveIfError(SendReceive(EMemSpyClientServerOpHeapInfoCompact)); 
	}

void RMemSpySession::SetOutputTypeL(TMemSpyOutputType aOutputType)
	{
	TInt operation = aOutputType == EOutputTypeFile ?
			EMemSpyClientServerOpSwitchOutputSinkFile :
			EMemSpyClientServerOpSwitchOutputSinkTrace;
	
	User::LeaveIfError(SendReceive(operation));
	}
