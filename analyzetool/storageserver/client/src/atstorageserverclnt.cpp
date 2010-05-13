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
* Description:  Definitions for the client side handle class RATStorageServer
*
*/



// INCLUDE FILES
#include    "atstorageserverclnt.h"
#include    "atstorageserver.h"
#include    "atlog.h"
#include    "atstorageservercommon.h"


// CONSTANTS

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// RATStorageServer::RATStorageServer
// C++ default constructor.
// -----------------------------------------------------------------------------
//
EXPORT_C RATStorageServer::RATStorageServer() :
    RSessionBase(), iStackBuf( NULL )
    {
    LOGSTR1( "STSE RATStorageServer::RATStorageServer()" );
    }

// -----------------------------------------------------------------------------
// RATStorageServer::Connect
// A function for connecting to the server
// -----------------------------------------------------------------------------
//
EXPORT_C TInt RATStorageServer::Connect()
    {
    LOGSTR1( "STSE TInt RATStorageServer::Connect()" );
 
    TInt errorCode( KErrNone );
    errorCode = CreateProcess();
    
    // If process's creation failed, return the error code
    if ( errorCode != KErrNone )
        {
        return errorCode;
        }
        
    // Otherwise create a new session    
    errorCode = CreateSession( KStorageServerName, Version() );
    
    // If creation of a new session failed, return the error code
    if ( errorCode != KErrNone )
        {
        return errorCode;
        }
    
    // Construct a buffer for call stack's memory addresses. If the function leaves,
    // the leave code is put in "errorCode". 
    TRAP( errorCode, ConstructBuffersL() ); 
    
    return errorCode;
    }

// -----------------------------------------------------------------------------
// RATStorageServer::Close
// Destroys all memory reserved by this class, and calls RSessionBase::Close()
// -----------------------------------------------------------------------------
//
EXPORT_C void RATStorageServer::Close()
    {
    LOGSTR1( "STSE void RATStorageServer::Close()" );
    
    // Delete iStackBuf
    delete iStackBuf;
    iStackBuf = NULL;
    
    // Call the base class' Close()
    RSessionBase::Close(); 
    }

// -----------------------------------------------------------------------------
// RATStorageServer::LogProcessStarted
// -----------------------------------------------------------------------------
//
EXPORT_C TInt RATStorageServer::LogProcessStarted( const TDesC& aFileName,
                                                   const TDesC8& aProcessName,
                                                   TUint aProcessId,
                                                   TUint32 aLogOption, TUint32 aIsDebug )
    {
    LOGSTR1( "STSE TInt RATStorageServer::LogProcessStarted()" );
    
    // Panic always if aFileName or aProcessName are longer than allowed.
    __ASSERT_ALWAYS( aFileName.Length() <= KMaxFileName &&
                     aProcessName.Length() <= KMaxProcessName,
        StorageServerPanic( KCategoryClient, EAToolBadArgument ) );
    
    // Panic (in debug builds) if aProcessId is KNullProcessId
    __ASSERT_DEBUG( aProcessId != KNullProcessId,
        StorageServerPanic( KCategoryClient, EAToolBadArgument ) );

    // This information is optional -> ignoring error
    // Must be before EProcessStarted in order to append udeb/urel
    // information to LogProcessStarted
    TIpcArgs ipcArgs2( aIsDebug );
    SendReceive( CATStorageServer::EProcessUdeb, ipcArgs2 );
    
    TIpcArgs ipcArgs( &aFileName, &aProcessName, aProcessId, aLogOption );
    TInt error( 0 );
    
    error = SendReceive( CATStorageServer::EProcessStarted, ipcArgs );

    // Return, if error is not KErrNone.
    if ( error != KErrNone )
        {
        return error;
        }
    
    return KErrNone;
    }
       
// -----------------------------------------------------------------------------
// RATStorageServer::LogDllLoaded
// -----------------------------------------------------------------------------
//
EXPORT_C TInt RATStorageServer::LogDllLoaded( const TDesC8& aDllName,
                                              TUint32 aStartAddress,
                                              TUint32 aEndAddress )
    {
    LOGSTR1( "STSE TInt RATStorageServer::LogDllLoaded()" );
    
    // Panic always if aDllName is longer than allowed.
    __ASSERT_ALWAYS( aDllName.Length() <= KMaxLibraryName,
        StorageServerPanic( KCategoryClient, EAToolBadArgument ) );
    
    // Panic (in debug builds) if aStartAddress is greater than aEndAddress
    __ASSERT_DEBUG( aStartAddress <= aEndAddress,
        StorageServerPanic( KCategoryClient, EAToolBadArgument ) );
    
    TIpcArgs ipcArgs( &aDllName, aStartAddress, aEndAddress );
    
    return SendReceive( CATStorageServer::EDllLoaded, ipcArgs );
    }
    
// -----------------------------------------------------------------------------
// RATStorageServer::LogDllUnloaded
// -----------------------------------------------------------------------------
//
EXPORT_C TInt RATStorageServer::LogDllUnloaded( const TDesC8& aDllName,
                                                TUint32 aStartAddress,
                                                TUint32 aEndAddress )
    {
    LOGSTR1( "STSE TInt RATStorageServer::LogDllUnloaded()" );
    
    // Panic always if aDllName is longer than allowed.
    __ASSERT_ALWAYS( aDllName.Length() <= KMaxLibraryName,
        StorageServerPanic( KCategoryClient, EAToolBadArgument ) );
    
    // Panic (in debug builds) if aStartAddress is greater than aEndAddress
    __ASSERT_DEBUG( aStartAddress <= aEndAddress,
        StorageServerPanic( KCategoryClient, EAToolBadArgument ) );
      
    TIpcArgs ipcArgs( &aDllName, aStartAddress, aEndAddress );
    
    return SendReceive( CATStorageServer::EDllUnloaded, ipcArgs );
    }    
        
// -----------------------------------------------------------------------------
// RATStorageServer::LogMemoryAllocated
// -----------------------------------------------------------------------------
//
EXPORT_C TInt RATStorageServer::LogMemoryAllocated( TUint32 aMemAddress,
                                                    TFixedArray<TUint32, KATMaxCallstackLength>& aCallstack,
                                                    TInt aSize )
    {
    LOGSTR1( "STSE TInt RATStorageServer::LogMemoryAllocated()" );
    
    TInt count=0;
    // The number of memory addresses in aCallstack
    for ( TInt i = 0; i < KATMaxCallstackLength; i++ )
        {
        if( aCallstack[i] == 0 )
            {
            break;
            }
        count++;
        }
    
    LOGSTR2( "STSE The number of memory addresses in the current call stack: %d", count );
    
    // The maximum number of call stack's memory addresses this server can handle
    // is KMaxCallstackLength
    if ( count > KATMaxCallstackLength )
        {
        count = KATMaxCallstackLength;
        }
        
    // Panic (in debug builds) if the parameters are faulty
    __ASSERT_DEBUG( aMemAddress != 0 && count >= 0 && aSize >= 0,
        StorageServerPanic( KCategoryClient, EAToolBadArgument ) );
          
    // Buffer position
    TInt pos = 0;  
    TUint32 callStackAddr;
                 
    // Write the number of memory addresses of aCallstack (one word) into
    // the beginning of the call stack buffer
    iStackBuf->Write( pos, &count, KWordSize );
    
    // Increase the position by one word
    pos += KWordSize; 
      
    // Write all the memory addresses of aCallStack into the buffer 
    for ( TInt i = 0; i < count; i++ )
        {
        callStackAddr = aCallstack[i];
        
        // Write the current memory address (the length of an address is one word)
        iStackBuf->Write( pos, &callStackAddr, KWordSize ); 
        
        // Move the pos variable one word (4 bytes) onwards.
        pos += KWordSize;        
        }  
    
    TPtr8 bufPtr( iStackBuf->Ptr(0) );       
     
    TIpcArgs ipcArgs( aMemAddress, &bufPtr, aSize );
     
    return SendReceive( CATStorageServer::EMemoryAllocated, ipcArgs );  
    }
        
// -----------------------------------------------------------------------------
// RATStorageServer::LogMemoryFreed
// -----------------------------------------------------------------------------
//
EXPORT_C TInt RATStorageServer::LogMemoryFreed( TUint32 aMemAddress,
        TFixedArray<TUint32, KATMaxFreeCallstackLength>& aFreeCallstack )
    {
    LOGSTR1( "STSE TInt RATStorageServer::LogMemoryFreed()" );
    
    TInt count=0;
    // The number of memory addresses in aFreeCallstack
    for ( TInt i = 0; i < KATMaxCallstackLength; i++ )
        {
        if( aFreeCallstack[i] == 0 )
            {
            break;
            }
        count++;
        }
    LOGSTR3( "STSE > aFreeCallstack.Count() ( %i ), address( %x )", 
    		count, aMemAddress );
    
    // The maximum number of call stack's memory addresses this server can handle
	// is KMaxCallstackLength
	if ( count > KATMaxFreeCallstackLength )
		{
		count = KATMaxFreeCallstackLength;
		}
		  
	// Buffer position
	TInt pos = 0;	
	TUint32 callStackAddr;
				 
	// Write the number of memory addresses of aFreeCallstack (one word) into
	// the beginning of the call stack buffer
	iStackBuf->Write( pos, &count, KWordSize );
	
	// Increase the position by one word
	pos += KWordSize; 
	  
	// Write all the memory addresses of aFreeCallstack into the buffer 
	for ( TInt i = 0; i < count; i++ )
		{
		callStackAddr = aFreeCallstack[i];
		
		// Write the current memory address (the length of an address is one word)
		iStackBuf->Write( pos, &callStackAddr, KWordSize ); 
		
		// Move the pos variable one word (4 bytes) onwards.
		pos += KWordSize;        
		}  
	
	TPtr8 bufPtr( iStackBuf->Ptr( 0 ) );	 
	TIpcArgs ipcArgs( aMemAddress, &bufPtr );
    	
    return SendReceive( CATStorageServer::EMemoryFreed, ipcArgs );
    }        
        
// -----------------------------------------------------------------------------
// RATStorageServer::LogProcessEnded
// -----------------------------------------------------------------------------
//
EXPORT_C TInt RATStorageServer::LogProcessEnded( TUint aProcessId,
                                                 TUint aHandleLeaks )
    {
    LOGSTR1( "STSE TInt RATStorageServer::LogProcessEnded()" );
    
    // Panic (in debug builds) if the parameters are faulty
    __ASSERT_DEBUG( aProcessId != KNullProcessId,        
        StorageServerPanic( KCategoryClient, EAToolBadArgument ) );
    
    TIpcArgs ipcArgs( aProcessId, aHandleLeaks );
    
    return SendReceive( CATStorageServer::EProcessEnded, ipcArgs );
    }

//-----------------------------------------------------------------------------
// RATStorageServer::Version()
// Returns the version number.
//-----------------------------------------------------------------------------
//
EXPORT_C TVersion RATStorageServer::Version() const
    {
    LOGSTR1( "STSE TInt RATStorageServer::Version()" );
    
    return TVersion(KLowestVersionNumberMaj, KLowestVersionNumberMin,
                                                KLowestVersionNumberBld);
    }

// -----------------------------------------------------------------------------
// RATStorageServer::CheckMemoryAddress
// -----------------------------------------------------------------------------
//
EXPORT_C TInt RATStorageServer::CheckMemoryAddress( TUint32 aMemAddress ) const
    {
    LOGSTR1( "STSE TInt RATStorageServer::CheckMemoryAddress()" );
    
    // Panic (in debug builds) if the parameters are faulty
    __ASSERT_DEBUG( aMemAddress != 0, 
            StorageServerPanic( KCategoryClient, EAToolBadArgument ) );
    
    TIpcArgs ipcArgs( aMemAddress );
    
    return SendReceive( CATStorageServer::EMemoryCheck, ipcArgs ); 
    }

// -----------------------------------------------------------------------------
// RATStorageServer::GetProcessesL
// -----------------------------------------------------------------------------
//    
EXPORT_C TInt RATStorageServer::GetProcessesL( RArray<TATProcessInfo>& aProcesses )
    {
    LOGSTR1( "STSE TInt RATStorageServer::GetProcessesL()" );
    
    TInt error( KErrNone );
    TInt sizeOfProcessInfo = sizeof( TATProcessInfo );
    
    // Reset the sent array
    aProcesses.Reset();
    
    // Calculate the length of the buffer to be constructed for processes.
    // One word will be reserved for the length of the array.
    TInt bufferLength = KWordSize + KATMaxProcesses * sizeOfProcessInfo; 
    
    CBufFlat* processBuf;
    // Construct processBuf and expand it before the beginning (index 0)
    processBuf = CBufFlat::NewL( bufferLength );
    CleanupStack::PushL( processBuf );
    processBuf->ExpandL( 0, bufferLength );
    
    TPtr8 bufPtr( processBuf->Ptr(0) );
    
    // Send the buffer to the server, which will fill it.
    TIpcArgs ipcArgs( &bufPtr );  
    error = SendReceive( CATStorageServer::EGetProcesses, ipcArgs ); 
    // Return with the error code if the operation failed
    if ( error != KErrNone )
        {
        CleanupStack::PopAndDestroy( processBuf );
        return error;
        }
    
    // A variable for the number of TATProcessInfo objects
    TInt count( 0 );
     // A variable for the position
    TInt pos( 0 );
    
    // Read the value for count 
    processBuf->Read( pos, &count, KWordSize );
    
    // Return if we got an illegal value for count     
    if ( count < 0 || count > KATMaxProcesses )
        {
        CleanupStack::PopAndDestroy( processBuf );
        return KErrGeneral;
        }    

    // Move the position one word onwards.    
    pos += KWordSize;

    TATProcessInfo processInfo;

    // Go through all TATProcessInfo objects sent to the server 
    for ( TInt j = 0; j < count; j++ )
        {
        // Read one of the TATProcessInfo objects stored in the buffer.
        processBuf->Read( pos, &processInfo, sizeOfProcessInfo );
       
        // Append this processInfo to the array
        error = aProcesses.Append( processInfo );

        if ( error != KErrNone )
            {
            CleanupStack::PopAndDestroy( processBuf );
            return error;
            }

        // Move the pos variable one word onwards.
        pos += sizeOfProcessInfo;
        }  
    
    CleanupStack::PopAndDestroy( processBuf );
    return error;
    }
        
// -----------------------------------------------------------------------------
// RATStorageServer::GetLoadedDllsL
// -----------------------------------------------------------------------------
//    
EXPORT_C TInt RATStorageServer::GetLoadedDllsL( TUint aProcessId, 
                                                RArray< TBuf8<KMaxLibraryName> >& aDlls )
    {
    LOGSTR1( "STSE TInt RATStorageServer::GetLoadedDllsL()" );
    
    // Panic (in debug builds) if aProcessId is faulty
    __ASSERT_DEBUG( aProcessId != KNullProcessId,        
        StorageServerPanic( KCategoryClient, EAToolBadArgument ) );
    
    TInt error( KErrNone );
    
    // Size of a DLL descriptor
    TInt sizeOfDllDesc = sizeof( TBuf8<KMaxLibraryName> );
    
    // Reset the sent array
    aDlls.Reset();
    
    // Calculate the length of the buffer to be constructed for DLL names.
    // One word will be reserved for the length of the array.
    TInt bufferLength = KWordSize + KATMaxDlls * sizeOfDllDesc; 
    
    CBufFlat* dllBuf;
    // Construct dllBuf and expand it before the beginning (index 0)
    dllBuf = CBufFlat::NewL( bufferLength );
    CleanupStack::PushL( dllBuf );
    dllBuf->ExpandL( 0, bufferLength );
    
    TPtr8 bufPtr( dllBuf->Ptr(0) );
    
    // Call the server with the given arguments
    TIpcArgs ipcArgs( aProcessId, &bufPtr );  
    error = SendReceive( CATStorageServer::EGetDlls, ipcArgs );
    
    // Return, if error is not KErrNone.
    if ( error != KErrNone )
        {
        CleanupStack::PopAndDestroy( dllBuf );      
        return error;
        }
    
    // A variable for the number of objects in the buffer
    TInt count( 0 );
    
     // A variable for the position
    TInt pos( 0 );
    
    // Read the value for count 
    dllBuf->Read( pos, &count, KWordSize );
    
    // Return if we got an illegal value for count
    if ( count < 0 || count > KATMaxDlls )
        {
        CleanupStack::PopAndDestroy( dllBuf );
        return KErrGeneral;
        }

    // Move the position one word onwards.    
    pos += KWordSize;

    TBuf8<KMaxLibraryName> dllName;

    // Go through all DLL names objects sent to the server 
    for ( TInt j = 0; j < count; j++ )
        {
        // Read one of the DLL names stored in the buffer.
        dllBuf->Read( pos, &dllName, sizeOfDllDesc );
       
        // Append this DLL name to the array
        error = aDlls.Append( dllName );

        if ( error != KErrNone )
            {
            return error;
            }

        // Move the pos variable one word onwards.
        pos += sizeOfDllDesc;
        }  
    
    CleanupStack::PopAndDestroy( dllBuf );
    return error;
    }    

// -----------------------------------------------------------------------------
// RATStorageServer::GetLoggingModeL
// -----------------------------------------------------------------------------
//    
EXPORT_C TInt RATStorageServer::GetLoggingModeL( TUint aProcessId,
                                                TATLogOption& aLoggingMode )
    {
    LOGSTR1( "STSE TInt RATStorageServer::GetLoggingModeL()" );
    
    // Panic (in debug builds) if aProcessId is illegal
    __ASSERT_DEBUG( aProcessId != KNullProcessId,        
        StorageServerPanic( KCategoryClient, EAToolBadArgument ) );
    
    TInt error(0);
    
    // The length of the buffer to be constructed for logging mode
    TInt bufferLength = KWordSize; 
    
    CBufFlat* loggingModeBuf;
    // Construct allocInfoBuf and expand it before the beginning (index 0)
    loggingModeBuf = CBufFlat::NewL( bufferLength );
    CleanupStack::PushL( loggingModeBuf );
    loggingModeBuf->ExpandL( 0, bufferLength );
    
    TPtr8 bufPtr( loggingModeBuf->Ptr(0) );
    
    // Call the server
    TIpcArgs ipcArgs( aProcessId, &bufPtr );  
    error = SendReceive( CATStorageServer::EGetLoggingMode, ipcArgs );
    
    // Return if an error occured.
    if ( error )
        {
        CleanupStack::PopAndDestroy( loggingModeBuf );
        return error;
        }
      
    // A variable for the position
    TInt pos( 0 );
    
    // Read the value for aNumber
    loggingModeBuf->Read( pos, &aLoggingMode, KWordSize );
    
    CleanupStack::PopAndDestroy( loggingModeBuf );
    
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// RATStorageServer::StartSubTest
// -----------------------------------------------------------------------------
//    
EXPORT_C TInt RATStorageServer::StartSubTest( TUint aProcessId, 
                                              const TDesC8& aSubtestId, TInt aHandleCount )
    {
    LOGSTR1( "STSE TInt RATStorageServer::StartSubTest()" );
    
    // Panic if the parameters are faulty
    __ASSERT_ALWAYS( aSubtestId.Length() <= KATMaxSubtestIdLength,        
        StorageServerPanic( KCategoryClient, EAToolBadArgument ) );
        
    // Panic (in debug builds) if the parameters are faulty
    __ASSERT_DEBUG( aProcessId != KNullProcessId,        
        StorageServerPanic( KCategoryClient, EAToolBadArgument ) );
        
    TIpcArgs ipcArgs( aProcessId, &aSubtestId, aHandleCount );
    
    return SendReceive( CATStorageServer::ESubtestStart, ipcArgs );
    }
    
// -----------------------------------------------------------------------------
// RATStorageServer::StopSubTest
// -----------------------------------------------------------------------------
//    
EXPORT_C TInt RATStorageServer::StopSubTest( TUint aProcessId, 
                                             const TDesC8& aSubtestId, TInt aHandleCount )
    {
    LOGSTR1( "STSE TInt RATStorageServer::StopSubTest()" );
    
    // Panic if the parameters are faulty
    __ASSERT_ALWAYS( aSubtestId.Length() <= KATMaxSubtestIdLength,        
        StorageServerPanic( KCategoryClient, EAToolBadArgument ) );
        
    // Panic (in debug builds) if the parameters are faulty
    __ASSERT_DEBUG( aProcessId != KNullProcessId,        
        StorageServerPanic( KCategoryClient, EAToolBadArgument ) );
        
    TIpcArgs ipcArgs( aProcessId, &aSubtestId, aHandleCount );
    
    return SendReceive( CATStorageServer::ESubtestStop, ipcArgs );
    }
    
// -----------------------------------------------------------------------------
// RATStorageServer::GetCurrentAllocsL
// -----------------------------------------------------------------------------
//    
EXPORT_C TInt RATStorageServer::GetCurrentAllocsL( TUint aProcessId, 
                                                  TUint32& aNumber,
                                                  TUint32& aSize   )
    {
    LOGSTR1( "STSE TInt RATStorageServer::GetCurrentAllocsL()" );
    
    // Panic (in debug builds) if aProcessId is illegal
    __ASSERT_DEBUG( aProcessId != KNullProcessId,        
        StorageServerPanic( KCategoryClient, EAToolBadArgument ) );
    
    TInt error(0);
    
    // The length of the buffer to be constructed for allocation number and size
    TInt bufferLength = KWordSize + KWordSize; 
    
    CBufFlat* allocInfoBuf;
    // Construct allocInfoBuf and expand it before the beginning (index 0)
    allocInfoBuf = CBufFlat::NewL( bufferLength );
    CleanupStack::PushL( allocInfoBuf );
    allocInfoBuf->ExpandL( 0, bufferLength );
    
    TPtr8 bufPtr( allocInfoBuf->Ptr(0) );
    
    // Call the server
    TIpcArgs ipcArgs( aProcessId, &bufPtr );  
    error = SendReceive( CATStorageServer::EGetCurrentAllocs, ipcArgs );
    
    // Return if error is not KErrNone.
    if ( error != KErrNone )
        {
        CleanupStack::PopAndDestroy( allocInfoBuf );
        return error;
        }
      
    // A variable for the position
    TInt pos( 0 );
    
    // Read the value for aNumber
    allocInfoBuf->Read( pos, &aNumber, KWordSize );
    
    pos += KWordSize;
    
    // Read the value for aSize
    allocInfoBuf->Read( pos, &aSize, KWordSize );
    
    CleanupStack::PopAndDestroy( allocInfoBuf );
    
    return KErrNone;
    }
                                                  
// -----------------------------------------------------------------------------
// RATStorageServer::GetMaxAllocsL
// -----------------------------------------------------------------------------
//    
EXPORT_C TInt RATStorageServer::GetMaxAllocsL( TUint aProcessId, 
                                              TUint32& aNumber,
                                              TUint32& aSize   )
    {
    LOGSTR1( "STSE TInt RATStorageServer::GetMaxAllocsL()" );
    
    // Panic (in debug builds) if aProcessId is illegal
    __ASSERT_DEBUG( aProcessId != KNullProcessId,        
        StorageServerPanic( KCategoryClient, EAToolBadArgument ) );
        
    TInt error(0);
     
    // The length of the buffer to be constructed for allocation number and size
    TInt bufferLength = KWordSize + KWordSize; 

    CBufFlat* allocInfoBuf;
    // Construct allocInfoBuf and expand it before the beginning (index 0)
    allocInfoBuf = CBufFlat::NewL( bufferLength );
    CleanupStack::PushL( allocInfoBuf );
    allocInfoBuf->ExpandL( 0, bufferLength );

    TPtr8 bufPtr( allocInfoBuf->Ptr(0) );

    // Call the server
    TIpcArgs ipcArgs( aProcessId, &bufPtr );
    error = SendReceive( CATStorageServer::EGetMaxAllocs, ipcArgs );

    // Return, if error is not KErrNone.
    if ( error != KErrNone )
        {
        CleanupStack::PopAndDestroy( allocInfoBuf );
        return error;
        }

    // A variable for the position
    TInt pos( 0 );

    // Read the value for aNumber
    allocInfoBuf->Read( pos, &aNumber, KWordSize );

    pos += KWordSize;

    // Read the value for aSize
    allocInfoBuf->Read( pos, &aSize, KWordSize );

    CleanupStack::PopAndDestroy( allocInfoBuf );

    return KErrNone;
    }
    
// -----------------------------------------------------------------------------
// RATStorageServer::StartSubTest
// An overloaded version without a PID parameter
// -----------------------------------------------------------------------------
//    
EXPORT_C TInt RATStorageServer::StartSubTest( const TDesC8& aSubtestId )
    {
    LOGSTR1( "STSE TInt RATStorageServer::StartSubTest()" );
    
    // Panic if the parameters are faulty
    __ASSERT_ALWAYS( aSubtestId.Length() <= KATMaxSubtestIdLength,        
        StorageServerPanic( KCategoryClient, EAToolBadArgument ) );
        
    TIpcArgs ipcArgs( &aSubtestId );
    
    return SendReceive( CATStorageServer::ESubtestStart2, ipcArgs );
    }
    
// -----------------------------------------------------------------------------
// RATStorageServer::StopSubTest
// An overloaded version without a PID parameter
// -----------------------------------------------------------------------------
//    
EXPORT_C TInt RATStorageServer::StopSubTest( const TDesC8& aSubtestId )
    {
    LOGSTR1( "STSE TInt RATStorageServer::StopSubTest()" );
    
    // Panic if the parameters are faulty
    __ASSERT_ALWAYS( aSubtestId.Length() <= KATMaxSubtestIdLength,        
        StorageServerPanic( KCategoryClient, EAToolBadArgument ) );
        
    TIpcArgs ipcArgs( &aSubtestId );
    
    return SendReceive( CATStorageServer::ESubtestStop2, ipcArgs );
    }

// -----------------------------------------------------------------------------
// RATStorageServer::CancelLogging
// -----------------------------------------------------------------------------
//    
EXPORT_C TInt RATStorageServer::CancelLogging( TUint aProcessId )
    {
    LOGSTR1( "STSE TInt RATStorageServer::CancelLogging()" );
    
    // Panic (in debug builds) if aProcessId is illegal
    __ASSERT_DEBUG( aProcessId != KNullProcessId,        
        StorageServerPanic( KCategoryClient, EAToolBadArgument ) );
    
    TIpcArgs ipcArgs( aProcessId );
    
    return SendReceive( CATStorageServer::ECancelLogging, ipcArgs );
    }

// -----------------------------------------------------------------------------
// RATStorageServer::CreateProcess
// Checks if the server already exists. If it doesn't, creates a new process
// for it.
// -----------------------------------------------------------------------------
//
TInt RATStorageServer::CreateProcess()
    {
    LOGSTR1( "STSE TInt RATStorageServer::CreateProcess()" );
    
    TInt errorCode( KErrNone );
 
    // Check if the server already exists
    TFindServer findServer( KStorageServerName );
    TFullName fullName;

    errorCode = findServer.Next( fullName );
    
    // Return KErrNone if the server is already running
    if ( errorCode == KErrNone )
        {
        return KErrNone;
        }
    
    // Otherwise initiate starting the server by creating first a new process for it.
    // The second argument of the Create() method call is an empty descriptor, because
    // we don't need to pass any data to the thread function of the new process's main
    // thread.
    RProcess process;
    errorCode = process.Create( KStorageServerFile, KNullDesC );
    
    // Return the error code if the creation of the process failed
    if ( errorCode != KErrNone )
        {
        return errorCode;
        }
    
    TRequestStatus status;
    process.Rendezvous( status );
    
    // If the status is not KRequestPending, abort the project creation
    // and return KErrGeneral
    if ( status != KRequestPending )
        {
        process.RendezvousCancel( status );
        process.Kill( KErrGeneral );
        process.Close();
        return KErrGeneral;
        }
    
    // Otherwise make the first thread of the new process eligible for execution
    process.Resume();
    
    // Wait for a signal from the server       
    User::WaitForRequest( status );
    
    // Close the process handle    
    process.Close();
    
    // Return the error code
    return status.Int();
    }

// -----------------------------------------------------------------------------
// RATStorageServer::ConstructBuffersL
// Allocates buffers from heap. Called when connecting to the server
// -----------------------------------------------------------------------------
//
void RATStorageServer::ConstructBuffersL()
    {
    LOGSTR1( "STSE void RATStorageServer::ConstructBuffersL()" );
    
    // Calculate the length of the buffer to be constructed for call stack.
    // One word will be reserved for the length of the array
    TInt bufferLength = ( (1 + KATMaxCallstackLength) * KWordSize  );
    
    // Construct iStackBuf and expand it before the beginning (index 0)
    iStackBuf = CBufFlat::NewL( bufferLength );
    iStackBuf->ExpandL( 0, bufferLength );
    }
    
// -----------------------------------------------------------------------------
// RATStorageServer::GetLoggingFileL
// -----------------------------------------------------------------------------
//    
EXPORT_C TInt RATStorageServer::GetLoggingFileL( TUint aProcessId,
                                                    TDes8& aFileName )
    {
    LOGSTR1( "STSE TInt RATStorageServer::GetLoggingFileL()" );
    
    // Panic (in debug builds) if aProcessId is illegal
    __ASSERT_DEBUG( aProcessId != KNullProcessId,        
        StorageServerPanic( KCategoryClient, EAToolBadArgument ) );
    
    TInt error(0);

    TBuf8<KMaxFileName> fileBuf;
    // Call the server
    TIpcArgs ipcArgs( aProcessId, &fileBuf );  
    error = SendReceive( CATStorageServer::EGetLoggingFile, ipcArgs );
    
    // Return, if an error occured.
    if ( error )
        {
        return error;
        }
      
    aFileName.Copy( fileBuf );
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// RATStorageServer::GetUdebL
// -----------------------------------------------------------------------------
//    
EXPORT_C TInt RATStorageServer::GetUdebL( TUint aProcessId, TUint32& aIsUdeb )
    {
    LOGSTR1( "STSE TInt RATStorageServer::GetUdebL()" );
    
    // Panic (in debug builds) if aProcessId is illegal
    __ASSERT_DEBUG( aProcessId != KNullProcessId,        
        StorageServerPanic( KCategoryClient, EAToolBadArgument ) );
    
    TInt error(0);
    TBuf8<KMaxVersionName> isUdeb;
    _LIT8( KUdeb, "UDEB" );
    // Call the server
    TIpcArgs ipcArgs( aProcessId, &isUdeb );  
    error = SendReceive( CATStorageServer::EGetUdeb, ipcArgs );
    
    // Return, if an error occured.
    if ( error != KErrNone )
        {
        return error;
        }
    
    if ( isUdeb.Compare( KUdeb() ) == 0 )
        {
        LOGSTR1( "STSE TInt RATStorageServer::GetUdebL() - Is UDEB" );    
        aIsUdeb = 1;    
        }
    else
        {
        LOGSTR1( "STSE TInt RATStorageServer::GetUdebL() - Is UREL" );
        aIsUdeb = 0;    
        }

    return KErrNone;
    }

// -----------------------------------------------------------------------------
// RATStorageServer::IsMemoryAdded
// -----------------------------------------------------------------------------
//
EXPORT_C TInt RATStorageServer::IsMemoryAdded( TUint32 aMemAddress )
    {
    LOGSTR1( "STSE TInt RATStorageServer::IsMemoryAdded()" );
        
    TIpcArgs ipcArgs( aMemAddress );    
    return SendReceive( CATStorageServer::EIsMemoryAdded, ipcArgs );
    }

//  End of File
