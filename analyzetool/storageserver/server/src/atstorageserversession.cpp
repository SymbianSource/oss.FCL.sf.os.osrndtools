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
* Description:  Definitions and constants for the class CATStorageServerSession
*
*/



// INCLUDE FILES
#include    <utf.h>
#include    <analyzetool/analyzetooltraceconstants.h>
#include    "atstorageserversession.h"
#include    "atstorageserver.h"
#include    "atstorageservercommon.h"
#include    "atmemoryentry.h"
#include    "atlog.h"
#include    "atdynprocessinfo.h"
#include    "atdriveinfo.h"

// CONSTANTS

// New file name start and end index.
const TInt KNameIndexStart = 1;
const TInt KNameIndexEnd = 100;

// ==================== MEMBER FUNCTIONS for TAllocInfo ========================

// -----------------------------------------------------------------------------
// TAllocInfo::TAllocInfo
// Implementation for the constructor of the class TAllocInfo
// -----------------------------------------------------------------------------
//
TAllocInfo::TAllocInfo( TUint32 aMemAddress, TInt aAllocSize ) :
    iMemAddress( aMemAddress ),
    iAllocSize( aAllocSize )
    {
    }


// ============== MEMBER FUNCTIONS for CATStorageServerSession =================

// -----------------------------------------------------------------------------
// CATStorageServerSession::CATStorageServerSession
// C++ default constructor. It Does not contain any code that
// might leave.
// -----------------------------------------------------------------------------
//
CATStorageServerSession::CATStorageServerSession( CATStorageServer& aStorageServer ) :
    iStorageServer( aStorageServer ),
    iError( 0 ),
    iLeakArray( KLeakArrayGranularity ),
    iProcessId( KNullProcessId ),
    iLoggingOngoing( EFalse ),
    iLogOption( KDefaultLoggingMode ),
    iCurAllocSize( 0 ),
    iMaxAllocs( 0 ),
    iMaxAllocSize( 0 ),
    iLogFile( KEmpty() ),
    iIsUdeb( 1 )
    {
    LOGSTR1( "STSE CATStorageServerSession::CATStorageServerSession()" );
    
    // Initialize iMicroSecondsAt1970
    TTime time( KJanuaryFirst1970 );
    iMicroSecondsAt1970 = time.Int64();
    
    // Increment the server's session count by one (1)
    iStorageServer.IncSessionCount();
    }

// -----------------------------------------------------------------------------
// CATStorageServerSession::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CATStorageServerSession::ConstructL()
    {
    // Intentionally left empty
    }

// -----------------------------------------------------------------------------
// CATStorageServerSession::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CATStorageServerSession* CATStorageServerSession::NewL( CATStorageServer& aStorageServer )
    { 
    CATStorageServerSession* self = NewLC( aStorageServer );
    CleanupStack::Pop( self );
    
    return self;
    }

// -----------------------------------------------------------------------------
// CATStorageServerSession::NewLC
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CATStorageServerSession* CATStorageServerSession::NewLC( CATStorageServer& aStorageServer )
    { 
    CATStorageServerSession* self = new ( ELeave ) CATStorageServerSession( aStorageServer );
    
    CleanupStack::PushL( self );
    self->ConstructL();
    return self;
    }

// -----------------------------------------------------------------------------
// CATStorageServerSession::~CATStorageServerSession
// Destructor
// -----------------------------------------------------------------------------
CATStorageServerSession::~CATStorageServerSession()
    {
    LOGSTR1( "STSE CATStorageServerSession::~CATStorageServerSession()" );
      
    // Empty the array and delete the referenced objects
    iLeakArray.ResetAndDestroy();
  
    // Close the leak array
    iLeakArray.Close();
    
    // Close the allocation info array
    iAllocInfoArray.Close();
    
    // Check if process closed abnormal
    if ( iProcessId != KNullProcessId && 
         iLoggingOngoing && iLogOption != EATLoggingOff && 
         iError != KErrNoMemory )
        {
        LogAbnormalEnd();
        }
    
    // Close the file and the handle to the file server
    CloseFsAndFile();
    
    // Remove the process with the current PID from the server's array of processes
    TRAP_IGNORE( iStorageServer.RemoveProcessL( iProcessId ) );
    
    // Decrement the server's session count by one (1)
    iStorageServer.DecSessionCount();
    }
    
// -----------------------------------------------------------------------------
// CATStorageServerSession::ServiceL
// This function is called by the client/server framework
// -----------------------------------------------------------------------------
//
void CATStorageServerSession::ServiceL( const RMessage2& aMessage )
    {
    LOGSTR1( "STSE void CATStorageServerSession::ServiceL()" );
    
    // If logging has been cancelled for this session, return immediately
    if( iLogOption == EATLoggingOff )
        {
        aMessage.Complete( KErrCancel );
        return;
        }
    
    switch ( aMessage.Function() )
        {
        case CATStorageServer::EProcessStarted:
            {
            // If logging is not ongoing, set the log option
            if( !iLoggingOngoing )
                {
                // Set the operation mode
                SetLogOption( aMessage );
                }

            switch ( iLogOption )
                {
                case EATLogToTrace:
                    {
                    iError = LogProcessStartTraceL( aMessage );
                    }
                break;
                
                case EATLogToFile:
                    {
                    iError = LogProcessStartedL( aMessage );
                    }
                break;
                
                default:
                    {
                    // Panic the client and set iError KErrCancel, because being
                    // here implies that an illegal log option has been given.
                    PanicClient( EAToolIllegalLogOption, aMessage );
                    iError = KErrCancel;
                    }
                break;
                }
            } 
        break;
        
        
        case CATStorageServer::EDllLoaded:
            {
            switch ( iLogOption )
                {
                case EATLogToTrace:
                    {
                    iError = LogDllLoadTraceL( aMessage );
                    }
                break;
                
                case EATLogToFile:
                    {
                    iError = LogDllLoadedL( aMessage );
                    }
                break;
                
                default:
                    {
                    // Panic the client and set iError KErrCancel, because being
                    // here implies that an illegal log option has been given.
                    PanicClient( EAToolIllegalLogOption, aMessage );
                    iError = KErrCancel;
                    }
                break;
                }
            }
        break;
        
        
        case CATStorageServer::EDllUnloaded:
            {
            switch ( iLogOption )
                {
                case EATLogToTrace:
                    {
                    iError = LogDllUnloadTraceL( aMessage );
                    }
                break;
                
                case EATLogToFile:
                    {
                    iError = LogDllUnloadedL( aMessage );
                    }
                break;
                
                default:
                    {
                    // Panic the client and set iError KErrCancel, because being
                    // here implies that an illegal log option has been given.
                    PanicClient( EAToolIllegalLogOption, aMessage );
                    iError = KErrCancel;
                    }
                break;
                }
            }
        break;
        
        
        case CATStorageServer::EMemoryAllocated:
            {
            switch ( iLogOption )
                {
                case EATLogToTrace:
                    {
                    iError = LogMemoryAllocTraceL( aMessage );
                    }
                break;
                
                case EATLogToFile:
                    {
                    iError = LogMemoryAllocatedL( aMessage );
                    }
                break;
                
                default:
                    {
                    // Panic the client and set iError KErrCancel, because being
                    // here implies that an illegal log option has been given.
                    PanicClient( EAToolIllegalLogOption, aMessage );
                    iError = KErrCancel;
                    }
                break;
                }
            }
        break;
        
        
        case CATStorageServer::EMemoryFreed:
            {
            switch ( iLogOption )
                {
                case EATLogToTrace:
                    {
                    iError = LogMemoryFreedTraceL( aMessage );
                    }
                break;
                
                case EATLogToFile:
                    {
                    iError = LogMemoryFreedL( aMessage );
                    }
                break;
                
                default:
                    {
                    // Panic the client and set iError KErrCancel, because being
                    // here implies that an illegal log option has been given.
                    PanicClient( EAToolIllegalLogOption, aMessage );
                    iError = KErrCancel;
                    }
                break;
                }
            }
        break;
        
        
        case CATStorageServer::EProcessEnded:
            {
            switch ( iLogOption )
                {
                case EATLogToTrace:
                    {
                    iError = LogProcessEndTraceL( aMessage );
                    }
                break;
                
                case EATLogToFile:
                    {
                    iError = LogProcessEndedL( aMessage );
                    }
                break;
                
                default:
                    {
                    // Panic the client and set iError KErrCancel, because being
                    // here implies that an illegal log option has been given.
                    PanicClient( EAToolIllegalLogOption, aMessage );
                    iError = KErrCancel;
                    }
                break;
                }
            }
        break;
        
        
        case CATStorageServer::EMemoryCheck:
            {
            switch ( iLogOption )
                {
                case EATLogToTrace:
                    {
                    iError = CheckMemoryAddressTrace( aMessage );
                    }
                break;
                
                case EATLogToFile:
                    {
                    iError = CheckMemoryAddressL( aMessage );
                    }
                break;
                
                default:
                    {
                    // Panic the client and set iError KErrCancel, because being
                    // here implies that an illegal log option has been given.
                    PanicClient( EAToolIllegalLogOption, aMessage );
                    iError = KErrCancel;
                    }
                break;
                }
            }
        break;
        
        
        case CATStorageServer::EGetProcesses:
            {
            iError = GetProcessesL( aMessage );
            }
        break;
        
        
        case CATStorageServer::EGetDlls:
            {
            iError = GetDllsL( aMessage );
            }
        break;
        
        case CATStorageServer::EGetLoggingMode:
            {
            iError = GetLoggingModeL( aMessage );
            }
        break;
        
        case CATStorageServer::ESubtestStart:
            {
            iError = StartSubtestL( aMessage );
            }
        break;
        
        case CATStorageServer::ESubtestStop:
            {
            iError = StopSubtestL( aMessage );
            }
        break;
        
        case CATStorageServer::ESubtestStart2:
            {
            iError = StartSubtest2L( aMessage );
            }
        break;
               
        case CATStorageServer::ESubtestStop2:
            {
            iError = StopSubtest2( aMessage );
            }
        break;
        
        case CATStorageServer::EGetCurrentAllocs:
            {
            iError = GetCurrentAllocsL( aMessage );
            }
        break;
        
        case CATStorageServer::EGetMaxAllocs:
            {
            iError = GetMaxAllocsL( aMessage );
            }
        break;
        
        case CATStorageServer::ECancelLogging:
            {
            iError = CancelLoggingL( aMessage );
            }
        break;

        case CATStorageServer::EGetUdeb:
            {
            iError = GetUdebL( aMessage );
            }
        break;
        
        case CATStorageServer::EGetLoggingFile:
            {
            iError = GetLoggingFileL( aMessage );
            }
        break;

        case CATStorageServer::EProcessUdeb:
            {
            SetUdeb( aMessage );
            }
        break;
        
        case CATStorageServer::EIsMemoryAdded:
            {
            iError = IsMemoryAdded( aMessage );
            LOGSTR2( "STSE > IsMemoryAdded err = %i", iError );
            }
        break;       
        
        default:
            {
            // Panic both the client and server, because being here implies
            // that there is an internal error in the client/server.
            PanicClient( EAToolBadRequest, aMessage );
            StorageServerPanic( KCategoryServer, EAToolBadRequest );
            }
        break;
            
        }
    
    // Complete the message, if it has not been already cancelled.
    if ( iError != KErrCancel )
        {
        // Log the error code. Only KErrNoMemory errors are logged.
        if ( iLogOption == EATLogToFile && iError == KErrNoMemory  )
            {
            HandleError( iError );
            }
        else if ( iLogOption == EATLogToTrace && iError == KErrNoMemory )
            {
            HandleErrorTrace( iError );
            }
               
        // Complete serving the message 
        aMessage.Complete( iError );
        }
    }

// -----------------------------------------------------------------------------
// CATStorageServerSession::LogProcessStartedL()
// Opens a logging file with the requested name and then writes information
// on process start into the file.
// -----------------------------------------------------------------------------
//
TInt CATStorageServerSession::LogProcessStartedL( const RMessage2& aMessage )
    {
    LOGSTR1( "STSE TInt CATStorageServerSession::LogProcessStartedL()" );
    
    // Panic the client and return, if this method has already been called for this
    // session object (and a logging file has been opened)
    if ( iLoggingOngoing )
        {
        PanicClient( EAToolNotAllowed, aMessage );
        return KErrCancel;
        }
    
    iError = KErrNone;
    
    LOGMEM;
    
    // READ THE FIRST ARGUMENT (descriptor)  
    
    // Length of the first argument (index 0)
    TInt length = aMessage.GetDesLength( 0 );
    
    LOGSTR2( "STSE length of the fileName: %i", length );
    
    // Return if errors
    if ( length == KErrArgument || length == KErrBadDescriptor )
        {
        return length;
        }
    
    // Construct a buffer for file name, and leave the pointer on Cleanup Stack
    HBufC* fileName = HBufC::NewLC( length );
    TPtr fileNamePtr( fileName->Des() );
     
    // Read the client side's descriptor at index 0
    iError = aMessage.Read( 0, fileNamePtr );
    
    if ( iError != KErrNone )
        {
        CleanupStack::PopAndDestroy( fileName );
        return iError;
        }
    
    // READ THE SECOND ARGUMENT (descriptor)  
    
    // Length of the second argument (index 1)   
    length = aMessage.GetDesLength( 1 );
    
    LOGSTR2( "STSE length of the processName: %i", length );
    
    // Return if errors
    if ( length == KErrArgument || length == KErrBadDescriptor )
        {
        CleanupStack::PopAndDestroy( fileName );
        return length;
        }
    
    HBufC8* processName = HBufC8::NewL( length );
    TPtr8 bufPtr( processName->Des() );

    // Read the client side's descriptor at index 1 
    iError = aMessage.Read( 1, bufPtr );
  
    if ( iError != KErrNone )
        {
        delete processName;
        CleanupStack::PopAndDestroy( fileName );
        return iError;
        }
    
    // READ THE THIRD ARGUMENT (integer, a process ID)    
    TInt processId = aMessage.Int2();
    
    // Open a file server session and a file. The file
    // will be opened with the name received from the client
    iError = OpenFsAndFile( *fileName, *processName );
    CleanupStack::PopAndDestroy( fileName );
    // Return without logging, if an error occured
    if ( iError != KErrNone )
        {
        // Delete the local objects
        delete processName;
        return iError;
        }
    
    // Get the home time for the configuration UI
    iTime.HomeTime();
    
    // Add the process into the server's array of processes
    iError = iStorageServer.AddProcessL( *processName,
                                        processId,
                                        this,
                                        iTime.Int64() );

    // Return without logging, if an error occured
    if ( iError )
        {
        // Remove, if something was added regardless of the error
        // However, we must not remove an existing process
        if ( iError != KErrAlreadyExists )
            {
            iStorageServer.RemoveProcessL( processId );
            }
        return iError;
        }
      
    // Make a buffer that will be logged into the opened logging file
    TBuf8<KProcessStartBufLength> loggingBuf;
    loggingBuf.Format( KProcessStart, processName, processId );
    
    delete processName;
    
    // Get the current universal time       
    TInt64 timeFrom1970( GetTime() );
    
    // Append the current time in the 64-bit (max 16 characters) hexadecimal text
    // format
    loggingBuf.AppendNum( timeFrom1970, EHex );
    
    // Append udeb/urel information to the process start
    loggingBuf.Append( KSpace );  
    loggingBuf.AppendNum( iIsUdeb, EHex );
    
    // Append trace version information
    loggingBuf.Append( KSpace );
    loggingBuf.AppendNum( KATTraceVersion, EHex );
    
    // Append a new line
    loggingBuf.Append( KNewLine );
    
    // Write the buffer into the file  
    iError = iFile.Write( loggingBuf );
    
    // Return, if an error occured
    if ( iError )
        {
        iStorageServer.RemoveProcessL( processId );
        return iError;
        }
    
    LOGMEM;
    
    // Set the process ID value for this logging session
    iProcessId = processId;
    // Set logging session started
    iLoggingOngoing = ETrue;
    
    return iError;
    }
    
// -----------------------------------------------------------------------------
// CATStorageServerSession::LogProcessStartTraceL()
// -----------------------------------------------------------------------------
//    
TInt CATStorageServerSession::LogProcessStartTraceL( const RMessage2& aMessage )
    {
    LOGSTR1( "STSE TInt CATStorageServerSession::LogProcessStartTraceL()" );
    
    // Panic the client and return, if this method has already been called for this
    // session object
    if ( iLoggingOngoing )
        {
        PanicClient( EAToolNotAllowed, aMessage );
        return KErrCancel;
        }
    
    iError = KErrNone;
    
    LOGMEM;
    
    // READ THE SECOND ARGUMENT (descriptor)
    // The first argument, file name, is ignored when logging thru trace  
    
    // Length of the second argument (index 1)
    TInt length = aMessage.GetDesLength( 1 );
    
    LOGSTR2( "STSE length of the processName: %i", length );

    // Return if errors
    if ( length == KErrArgument || length == KErrBadDescriptor )
        {
        return length;
        }

    HBufC8* processName = HBufC8::NewL( length );
    TPtr8 bufPtr( processName->Des() );

    // Read the client side's descriptor at index 1 
    iError = aMessage.Read( 1, bufPtr );
  
    if ( iError != KErrNone )
        {
        // Delete local objects and return
        delete processName;
        return iError;
        }
    
    // READ THE THIRD ARGUMENT (integer, a process ID)    
    TInt processId = aMessage.Int2();

    // Get the home time for the configuration UI
    iTime.HomeTime();
    
    // Add the process into the server's array of processes
    iError = iStorageServer.AddProcessL( *processName, processId, this,
                                             iTime.Int64() );
    
    // Return without logging, if an error occured
    if ( iError )
        {
        // Remove, if something was added regardless of the error
        // However, we must not remove an existing process
        if ( iError != KErrAlreadyExists )
            {
            iStorageServer.RemoveProcessL( processId );
            }
        return iError;
        }

    // Make a buffer that will be logged
    TBuf8<KProcessStartBufLength> loggingBuf;
    
    loggingBuf.Format( KProcessStart, processName, processId );
    
    delete processName;
    
    // Get the current universal time      
    TInt64 timeFrom1970( GetTime() );
    
    // Append the current time in the 64-bit (max 16 characters) hexadecimal text
    // format
    loggingBuf.AppendNum( timeFrom1970, EHex );
    
    // Append udeb/urel information to the process start
    loggingBuf.Append( KSpace );
    loggingBuf.AppendNum( iIsUdeb, EHex );
    
    // Append version number
    loggingBuf.Append( KSpace );  
    loggingBuf.AppendNum( KATTraceVersion, EHex );
    
    // Append a new line
    loggingBuf.Append( KNewLine );
    
    // Log to trace
    TBuf<KProcessStartBufLength> traceBuf;
    traceBuf.Copy( loggingBuf );
    RDebug::Print( KTraceMessage, processId ,&traceBuf );
    
    LOGMEM;
    
    // Set the process ID value for this logging session
    iProcessId = processId;
    // Set logging session started
    iLoggingOngoing = ETrue;
    
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CATStorageServerSession::LogDllLoadedL()
// Logs to the file opened by the function LogProcessStartedL()
// -----------------------------------------------------------------------------
//
TInt CATStorageServerSession::LogDllLoadedL( const RMessage2& aMessage )
    {
    LOGSTR1( "STSE TInt CATStorageServerSession::LogDllLoadedL()" );
    
    // Panic the client and return, if a logging session is not ongoing
    // ( can be started by calling the client's LogProcessStarted() )
    if ( !iLoggingOngoing )
        {
        PanicClient( EAToolNotAllowed, aMessage );
        return KErrCancel;
        }
    
    iError = KErrNone;
    
    // Read the length of the first argument (index 0)   
    TInt length = aMessage.GetDesLength( 0 );
    
    // Return if errors
    if ( length == KErrArgument || length == KErrBadDescriptor )
        {
        return length;
        }
    
    HBufC8* dllName = HBufC8::NewL( length );
    TPtr8 bufPtr( dllName->Des() );

    // Read the client side's descriptor (the argument 0) 
    iError = aMessage.Read( 0, bufPtr );
  
    if ( iError != KErrNone )
        {
        delete dllName;
        return iError;
        }
    
    // Get the current universal time
	TInt64 timeFrom1970( GetTime() );
        
    // Add this dll into the server's array
    TUint32 startAddress( aMessage.Int1() );        
    TUint32 endAddress( aMessage.Int2() );
    iError = iStorageServer.AddDllL( iProcessId, 
    		TATDllInfo( startAddress, endAddress, timeFrom1970, *dllName ) );
    
    // Return without logging, if an error occured
    if ( iError )
        {
        delete dllName;
        return iError;
        }
    
    // Make a buffer that will be logged into the opened logging file
    TBuf8<KDllLoadBufLength> loggingBuf;
    loggingBuf.Format( KDllLoad, dllName, timeFrom1970, startAddress, endAddress );
    
    delete dllName;
    
    // Write the buffer into a file and return the error code   
    return iFile.Write( loggingBuf );
    }
    
// -----------------------------------------------------------------------------
// CATStorageServerSession::LogDllLoadTraceL()
// -----------------------------------------------------------------------------
//    
TInt CATStorageServerSession::LogDllLoadTraceL( const RMessage2& aMessage )
    {
    LOGSTR1( "STSE TInt CATStorageServerSession::LogDllLoadTraceL()" );
    
    // Panic the client and return, if a logging session is not ongoing
    // ( can be started by calling the client's LogProcessStarted() )
    if ( !iLoggingOngoing )
        {
        PanicClient( EAToolNotAllowed, aMessage );
        return KErrCancel;
        }
    
    iError = KErrNone;
    
    // Read the length of the first argument (index 0)
    TInt length = aMessage.GetDesLength( 0 );
    
    // Return if errors
    if ( length == KErrArgument || length == KErrBadDescriptor )
        {
        return length;
        }
    
    HBufC8* dllName = HBufC8::NewL( length );
    TPtr8 bufPtr( dllName->Des() );

    // Read the client side's descriptor (the argument 0) 
    iError = aMessage.Read( 0, bufPtr );
  
    if ( iError != KErrNone )
        {
        delete dllName;
        return iError;
        }
    // Get the current universal time
	TInt64 timeFrom1970( GetTime() );
    	
    TUint32 startAddress( aMessage.Int1() );
    TUint32 endAddress( aMessage.Int2() );
        
    // Add this dll into the server's array 
    iError = iStorageServer.AddDllL( iProcessId, 
    		TATDllInfo( startAddress, endAddress, timeFrom1970, *dllName ) );
    
    // Return without logging, if an error occured
    if ( iError )
        {
        delete dllName;
        return iError;
        }
    
    // Make a buffer that will be logged
    TBuf8<KDllLoadBufLength> loggingBuf;
    loggingBuf.Format( KDllLoad, dllName, timeFrom1970, startAddress, endAddress );
    
    delete dllName;
    
    TBuf<KDllLoadBufLength> traceBuf;
    traceBuf.Copy( loggingBuf );
    RDebug::Print( KTraceMessage, iProcessId ,&traceBuf );
    return iError;
    }
    
// -----------------------------------------------------------------------------
// CATStorageServerSession::LogDllUnloadedL()
// Logs to the file opened by the function LogProcessStartedL()
// -----------------------------------------------------------------------------
//
TInt CATStorageServerSession::LogDllUnloadedL( const RMessage2& aMessage )
    {
    LOGSTR1( "STSE TInt CATStorageServerSession::LogDllUnloadedL()" );
    
    // Panic the client and return, if a logging session is not ongoing
    // ( can be started by calling the client's LogProcessStarted() )
    if ( !iLoggingOngoing )
        {
        PanicClient( EAToolNotAllowed, aMessage );
        return KErrCancel;
        }
    
    iError = KErrNone;
    
    // Read the length of the first argument (index 0)   
    TInt length = aMessage.GetDesLength( 0 );
    
    LOGSTR2( "STSE length %i", length );
    
    // Return if errors
    if ( length == KErrArgument || length == KErrBadDescriptor )
        {
        return length;
        }
    
    HBufC8* dllName = HBufC8::NewL( length );
    TPtr8 bufPtr( dllName->Des() );

    // Read the client side's descriptor (the argument 0) 
    iError = aMessage.Read( 0, bufPtr );
  
    if ( iError != KErrNone )
        {
        delete dllName;
        return iError;
        }
       
    TUint32 startAddress = aMessage.Int1();
    TUint32 endAddress = aMessage.Int2();
    
    // Get the current universal time
    TInt64 timeFrom1970( GetTime() );
        
    // Make a buffer that will be logged into the opened logging file
    TBuf8<KDllUnloadBufLength> loggingBuf;
    loggingBuf.Format( KDllUnload, dllName, timeFrom1970, startAddress, endAddress );
    
    // Remove this dll from the server's array
    iError = iStorageServer.RemoveDllL( iProcessId, bufPtr );
    
    delete dllName;
    
    // Return without logging, if an error occured
    if ( iError )
        {
        return iError;
        }
     
    // Write the buffer into a file and return the error code   
    return iFile.Write( loggingBuf );
    }
 
// -----------------------------------------------------------------------------
// CATStorageServerSession::LogDllUnloadTraceL()
// -----------------------------------------------------------------------------
//    
TInt CATStorageServerSession::LogDllUnloadTraceL( const RMessage2& aMessage )
    {
    LOGSTR1( "STSE TInt CATStorageServerSession::LogDllUnloadTraceL()" );
    
    // Panic the client and return, if a logging session is not ongoing
    // ( can be started by calling the client's LogProcessStarted() )
    if ( !iLoggingOngoing )
        {
        PanicClient( EAToolNotAllowed, aMessage );
        return KErrCancel;
        }
        
    iError = KErrNone;
    
    // Read the length of the first argument (index 0)   
    TInt length = aMessage.GetDesLength( 0 );
    
    LOGSTR2( "STSE length %i", length );
    
    // Return if errors
    if ( length == KErrArgument || length == KErrBadDescriptor )
        {
        return length;
        }
    
    HBufC8* dllName = HBufC8::NewL( length );
    TPtr8 bufPtr( dllName->Des() );

    // Read the client side's descriptor (the argument 0) 
    iError = aMessage.Read( 0, bufPtr );
  
    if ( iError != KErrNone )
        {
        delete dllName;
        return iError;
        }
    
    TUint32 startAddress = aMessage.Int1();
    TUint32 endAddress = aMessage.Int2();
    
    // Get the current universal time
    TInt64 timeFrom1970( GetTime() );
        
    // Make a buffer that will be logged
    TBuf8<KDllUnloadBufLength> loggingBuf;
    loggingBuf.Format( KDllUnload, dllName, timeFrom1970, startAddress, endAddress );
    
    // Remove this dll from the server's array
    iError = iStorageServer.RemoveDllL( iProcessId, bufPtr );
    
    delete dllName;
    
    // Return without logging, if an error occured
    if ( iError )
        {
        return iError;
        }
    
    TBuf<KDllLoadBufLength> traceBuf;
    traceBuf.Copy( loggingBuf );
    RDebug::Print( KTraceMessage, iProcessId ,&traceBuf );
    return iError;
    }
    
// -----------------------------------------------------------------------------
// CATStorageServerSession::LogMemoryAllocatedL()
// Constructs a CATMemoryEntry object and appends it into iLeakArray.
// -----------------------------------------------------------------------------
//
TInt CATStorageServerSession::LogMemoryAllocatedL( const RMessage2& aMessage )
    {
    LOGSTR1( "STSE TInt CATStorageServerSession::LogMemoryAllocatedL()" );
    
    // Panic the client and return, if a logging session is not ongoing
    // ( can be started by calling the client's LogProcessStarted() )
    if ( !iLoggingOngoing )
        {
        PanicClient( EAToolNotAllowed, aMessage );
        return KErrCancel;
        }     
    
    // A pointer to a buffer of call stack's memory addresses
    CBufFlat* stackBuf = NULL;
    
    iError = KErrNone;
    
    // Get the current universal time
    TInt64 timeFrom1970( GetTime() );
    
    // Read the first argument (index 0)
    TUint32 memAddress = aMessage.Int0();
    if ( memAddress == 0 )
        {
        return KErrNotSupported;
        }
    
    // Read the length of the descriptor argument (index 1) that should include
    // call stack memory addresses associated with this memory allocation
    TInt bufferLength = aMessage.GetDesLength( 1 );
    
    // Construct a buffer for aCallstack
    stackBuf = CBufFlat::NewL( bufferLength );
    CleanupStack::PushL( stackBuf );
    
    // Buffer position
    TInt pos = 0;
    
    stackBuf->ExpandL( pos, bufferLength );

    TPtr8 bufPtr = stackBuf->Ptr( pos );

    // Read the descriptor argument into the buffer
    aMessage.ReadL( 1, bufPtr );

    // Read the third argument (index 2) that tells the size of this allocation
    TInt size = aMessage.Int2();
    
    // Construct a new CATMemoryEntry object.   
    // The ownership of the current stackBuf object is given to the "entry" object.
    CATMemoryEntry* entry = 
        new (ELeave) CATMemoryEntry( memAddress, stackBuf, timeFrom1970, size );
    
    // Pop stackBuf from CleanupStack and set it to NULL, because it is not used anymore.
    CleanupStack::Pop( stackBuf );
    stackBuf = NULL;
    
    // Make sure that the same memory area is not tryed to be allocated a second time
    TIdentityRelation<CATMemoryEntry> matcher( CATMemoryEntry::Match );

    TInt index = iLeakArray.Find( entry, matcher );
    
    if ( index == KErrNotFound )
        {
        TLinearOrder<CATMemoryEntry> order( CATMemoryEntry::Compare );
        
        // Insert the "entry" object into "iLeakArray". The ownership of
        // the "entry" object is given to the array.
        iError = iLeakArray.InsertInOrderAllowRepeats( entry, order );
               
        // If an insertion to the array was not successful, delete the created
        // entry manually and return.
        if ( iError )
            {
            delete entry;
            return iError;
            }
        
        // Make a TAllocInfo object, and give values for its members.
        TAllocInfo allocInfo( memAddress, size );

        // Insert the allocInfo object into iAllocInfoArray
        iError = iAllocInfoArray.InsertInUnsignedKeyOrder( allocInfo );
    
        // If an insertion to the array was not successful, delete the created entry
        // and remove its pointer from iLeakArray.
        if ( iError )
            {
            index = iLeakArray.Find( entry, matcher );
            // Delete the entry object and remove remove the pointer from the array
            delete entry;
            // The index should be in a legal range, because the earlier insertion of
            // the entry was successful
            iLeakArray.Remove( index );
            }
        
        // Otherwise update the iCurAllocSize, iMaxAllocs and iMaxAllocSize variables
        
        iCurAllocSize += size;
        
        // The count can never be negative => associate it to an unsigned int
        TUint allocCount = iAllocInfoArray.Count();
        if ( allocCount > iMaxAllocs )
            {
            iMaxAllocs = allocCount;
            }
        
        if ( iCurAllocSize > iMaxAllocSize )
            {
            iMaxAllocSize = iCurAllocSize;
            }
        
        return iError;
        }
        
    // This shouldn't happen, because the same memory area shouldn't be allocated
    // more than once (without deallocating it first)
    else
        {
        delete entry;
        return KErrAlreadyExists;
        }
    }
    
// -----------------------------------------------------------------------------
// CATStorageServerSession::LogMemoryAllocTraceL()
// -----------------------------------------------------------------------------
//    
TInt CATStorageServerSession::LogMemoryAllocTraceL( const RMessage2& aMessage )
    {
    LOGSTR1( "STSE TInt CATStorageServerSession::LogMemoryAllocTraceL()" );
    
    // Panic the client and return, if a logging session is not ongoing
    // ( can be started by calling the client's LogProcessStarted() )
    if ( !iLoggingOngoing )
        {
        PanicClient( EAToolNotAllowed, aMessage );
        return KErrCancel;
        }
    
    // Read the first argument (index 0)
    TUint32 memAddress = aMessage.Int0();
    if ( memAddress == 0 )
        {
        return KErrNotSupported;
        }
    
    // Read the third argument (index 2) that tells the size of this allocation
    TInt size = aMessage.Int2();
    
    // Append this allocation into the iAllocInfoArray array. This array is for
    // providing the configuration UI with information on allocations
    
    // Make a TAllocInfo object, and give values for its members.
    TAllocInfo allocInfo( memAddress, size );

    // Insert the allocInfo object into iAllocInfoArray
    iError = iAllocInfoArray.InsertInUnsignedKeyOrder( allocInfo );
    
    // Log debug message if duplicated allocation.
    if ( iError == KErrAlreadyExists )
        {
        LOGSTR2( "STSE TInt CATStorageServerSession::LogMemoryAllocTraceL() Error, duplicate allocation :%i", memAddress );
        }

    // A pointer to a buffer of call stack's memory addresses
    CBufFlat* stackBuf = NULL;
    
    // Get the current universal time           
    TInt64 timeFrom1970( GetTime() );     
    
    // Read the length of the descriptor argument (index 1) that should include
    // call stack memory addresses associated with this memory allocation
    TInt bufferLength = aMessage.GetDesLength( 1 );
    
    // Construct a buffer for aCallstack
    stackBuf = CBufFlat::NewL( bufferLength );
    CleanupStack::PushL( stackBuf );
    
    // Buffer position
    TInt pos( 0 );    
    stackBuf->ExpandL( pos, bufferLength );
    
    TPtr8 bufPtr = stackBuf->Ptr( pos );
    
    // Read the descriptor argument (index 1) into the buffer
    aMessage.ReadL( 1, bufPtr );
    
    // Variable for the number of memory addresses in the call stack
    TInt addrCount( 0 );    
    TUint32 callStackAddr;
    
    // Read the first word of the buffer. This includes the number of
    // memory addresses stored in the current stackBuf   
    stackBuf->Read( pos, &addrCount, KWordSize );
    
    // Move the position one word onwards.    
    pos += KWordSize;
    
    // Create a 16-bit buffer, and a pointer descriptor for it    
    // ALLOCH <Memory address> <Time stamp> <Allocation size> <Call stack address count> 
    // <Call stack address> <Call stack address> ...
    HBufC* traceBuf = HBufC::NewL( KMemAllocBufLength );
    TPtr tracePtr( traceBuf->Des() );
    
    // Pop stackBuf from CleanupStack, since no leavable operations will be done
    // anymore
    CleanupStack::Pop( stackBuf );
    
    // Append the tag implying a memory allocation line in the data file
    tracePtr.Append( KMemoryAllocHeader );
    
    // Append the start address of this allocation in the 32-bit (max 8 characters)
    // hexadecimal text format.
    tracePtr.AppendNum( memAddress, EHex );
    
    // Append the current time in the 64-bit (max 16 characters) hexadecimal text
    // format
    tracePtr.Append( KSpaceTrace );
    tracePtr.AppendNum( timeFrom1970, EHex );
    
    // Append the size of the allocation in the 32-bit (max 8 characters) hexadecimal
    // text format.
    tracePtr.Append( KSpaceTrace );
    tracePtr.AppendNum( size, EHex );
    
    // Append call stack address count
    tracePtr.Append( KSpaceTrace );
    tracePtr.AppendNum( addrCount, EHex );
    
    // Calculate last item length
    TInt lastItemLength( KTraceMessage().Length() + KHexa32Length + 
            KSpaceLength + KNewlineLength );
    
    TUint packetNumber( 1 );
    
    // Go through all call stack's memory addresses associated with
    // this memory allocation 
    for ( TInt j = 0; j < addrCount; j++ )
        {
        // ALLOCF <Memory address> <Time stamp> <Packet number> 
        // <Call stack address> <Call stack address> ...
        if ( tracePtr.Length() <= 0 )
            {                
            // Create alloc fragment message header
            tracePtr.Append( KMemoryAllocFragment );
            tracePtr.AppendNum( memAddress, EHex );
            tracePtr.Append( KSpaceTrace );
            tracePtr.AppendNum( timeFrom1970, EHex );
            tracePtr.Append( KSpaceTrace );        
            tracePtr.AppendNum( packetNumber, EHex );
            // Increase packet number
            packetNumber++;
            }
        
        // Read the next call stack's memory address stored in the buffer.
        stackBuf->Read( pos, &callStackAddr, KWordSize );
        
        // Append the read memory address as a hexadecimal number
        tracePtr.AppendFormat( KHexaNumberTrace, callStackAddr );
        
        // Move the pos variable one word onwards.
        pos += KWordSize;
        
        // Check if buffer max length exceed
        if ( lastItemLength + tracePtr.Length() >= KMemAllocBufLength )
            {
            tracePtr.Append( KNewLineTrace );
            // Log through debug channel 
            RDebug::Print( KTraceMessage, iProcessId, traceBuf );
            // Empty trace buffer
            tracePtr.Delete( 0, tracePtr.MaxLength() );
            }
        }
    
    // Send the last message if exists
    if ( tracePtr.Length() > 0 )
        {
        tracePtr.Append( KNewLineTrace );
        
        // Log through debug channel 
        RDebug::Print( KTraceMessage, iProcessId, traceBuf );
        }
    
    delete traceBuf;
    delete stackBuf;
    
    // Update the iCurAllocSize, iMaxAllocs and iMaxAllocSize variables            
    iCurAllocSize += size;
    
    // The count can never be negative => associate it to an unsigned int
    TUint allocCount = iAllocInfoArray.Count();
    if ( allocCount > iMaxAllocs )
        {
        iMaxAllocs = allocCount;
        }
    
    if ( iCurAllocSize > iMaxAllocSize )
        {
        iMaxAllocSize = iCurAllocSize;
        }
    
    return KErrNone;    
    }    
    
// -----------------------------------------------------------------------------
// CATStorageServerSession::LogMemoryFreedL()
// Removes a TATMemoryEntry object with the specified memory address from 
// iLeakArray, if found.
// -----------------------------------------------------------------------------
//
TInt CATStorageServerSession::LogMemoryFreedL( const RMessage2& aMessage )
    {
    LOGSTR1( "STSE TInt CATStorageServerSession::LogMemoryFreedL()" );
    
    // Panic the client and return, if a logging session is not ongoing
    // ( can be started by calling the client's LogProcessStarted() )
    if ( !iLoggingOngoing )
        {
        PanicClient( EAToolNotAllowed, aMessage );
        return KErrCancel;
        }
    
    // Get the memory address
    TUint32 memAddress = aMessage.Int0();
    
    // Remove this memory allocation from the leak array
    TIdentityRelation<CATMemoryEntry> matcher( CATMemoryEntry::Match );
    CATMemoryEntry* entry = new (ELeave) CATMemoryEntry( memAddress, NULL, 0, 0 );
    TInt index = iLeakArray.Find( entry, matcher );
    delete entry;
    
    // Return, if the requested memory address was not found
    // (had not been allocated)
    if ( index == KErrNotFound )
        {
        return index;
        }
    
    // Delete the CATMemoryEntry object at "index" and remove from the array
    delete iLeakArray[index];
    iLeakArray.Remove( index );
    
    // Remove this memory allocation also from the allocation info array
    // Make a TAllocInfo object for a "find" operation
    TAllocInfo allocInfo( memAddress, 0 );
    index = iAllocInfoArray.FindInUnsignedKeyOrder( allocInfo );
    
    // The index should not be KErrNotFound, because an object with this memory address
    // was found in the iLeakArray array. If the index is out of range, something is
    // badly wrong, so it would be alright to panic in that case.
    if ( index == KErrNotFound )
        {
        PanicClient( EAToolInternalError, aMessage );
        return KErrCancel;
        }
    
    // Decrease the current alloc size and remove the requested allocation
    // from iAllocInfoArray
    iCurAllocSize -= iAllocInfoArray[index].iAllocSize;
    iAllocInfoArray.Remove( index );
    
    // If we are here, everything has gone alright
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CATStorageServerSession::IsMemoryAdded()
// Check a memory allocation (memory address) from an internal array.
// -----------------------------------------------------------------------------
//    
TInt CATStorageServerSession::IsMemoryAdded( const RMessage2& aMessage, 
    const TBool aRemoveAlloc )
    {
    LOGSTR1( "STSE TInt CATStorageServerSession::IsMemoryAdded()" );
    
    // Panic the client and return, if a logging session is not ongoing
    // ( can be started by calling the client's LogProcessStarted() )
    if ( !iLoggingOngoing )
        {
        PanicClient( EAToolNotAllowed, aMessage );
        return KErrCancel;
        }
        
    // Read the first argument (index 0)
    TUint32 memAddress = aMessage.Int0();
    
    // Try to find this memory allocation from the allocation info array
    
    // Make a TAllocInfo object for a "find" operation
    TAllocInfo allocInfo( memAddress, 0 );
    TInt index( iAllocInfoArray.FindInUnsignedKeyOrder( allocInfo ) );
    
    if ( index == KErrNotFound )
        {
        return index;
        }
    else if ( aRemoveAlloc )
        {
        // Otherwise decrease the current alloc size and remove the requested allocation
        // from iAllocInfoArray
        iCurAllocSize -= iAllocInfoArray[index].iAllocSize;
        iAllocInfoArray.Remove( index );
        }
    
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CATStorageServerSession::LogMemoryFreedTraceL()
// -----------------------------------------------------------------------------
//    
TInt CATStorageServerSession::LogMemoryFreedTraceL( const RMessage2& aMessage )
    {
    LOGSTR1( "STSE TInt CATStorageServerSession::LogMemoryFreedTraceL()" );
    
    // Panic the client and return, if a logging session is not ongoing
    // ( can be started by calling the client's LogProcessStarted() )
    if ( !iLoggingOngoing )
        {
        PanicClient( EAToolNotAllowed, aMessage );
        return KErrCancel;
        }
    
    // A pointer to a buffer of call stack's memory addresses
    CBufFlat* stackBuf = NULL;        
    iError = KErrNone;    
    
    // Read the first argument (index 0)
    TUint32 memAddress = aMessage.Int0();

    // Remove address from allocation table and its size from alloc size,
    // if found from table.
    TAllocInfo allocInfo( memAddress, 0 ); // Dummy info for search.
    TInt index( iAllocInfoArray.FindInUnsignedKeyOrder( allocInfo ) );
    if ( index != KErrNotFound )
        {
        // Decrease the current alloc size and remove the requested allocation
        // from table.
        iCurAllocSize -= iAllocInfoArray[index].iAllocSize;
        iAllocInfoArray.Remove( index );
        }
    else
        {
        LOGSTR2( "STSE TInt CATStorageServerSession::LogMemoryFreedTrace() Error, cannot find alloc for free: %i", memAddress );
        }
    
    // Read the length of the descriptor argument (index 1) that should include
    // call stack memory addresses associated with this memory allocation
    TInt bufferLength = aMessage.GetDesLength( 1 );
    
    // Construct a buffer for aCallstack
    stackBuf = CBufFlat::NewL( bufferLength );
    CleanupStack::PushL( stackBuf );
    
    // Buffer position
    TInt pos = 0;
    
    stackBuf->ExpandL( pos, bufferLength );

    TPtr8 bufPtr = stackBuf->Ptr( pos );

    // Read the descriptor argument (index 1) into the buffer
    aMessage.ReadL( 1, bufPtr );
    
    // Variable for the number of memory addresses in the call stack
    TInt addrCount( 0 );    
    TUint32 callStackAddr( 0 );

    // Read the first word of the buffer. This includes the number of
    // memory addresses stored in the current stackBuf
    stackBuf->Read( pos, &addrCount, KWordSize );

    // Move the position one word onwards.
    pos += KWordSize;
    
    // Create a 16-bit buffer, and a pointer descriptor for it
    HBufC* traceBuf = HBufC::NewL( KMemFreedBufLength );
    TPtr tracePtr( traceBuf->Des() );
    
    // Pop stackBuf from CleanupStack, since no leavable operations will be done
    // anymore
    CleanupStack::Pop( stackBuf );
    
	// Get the current universal time		
	TInt64 timeFrom1970( GetTime() );
			
    // Memory deallocation header message.
    // FREEH <Memory address> <Time stamp> <Call stack address count> <Call stack address>
    // <Call stack address> ...

    // Append the tag implying a memory free line in the data file
    tracePtr.Append( KMemoryFreedHeader );
    
    // Append the start address of this allocation in the 32-bit (max 8 characters)
    // hexadecimal text format.
    tracePtr.AppendNum( memAddress, EHex );    
    
	// Append the current time in the 64-bit (max 16 characters) hexadecimal text
	// format
	tracePtr.Append( KSpaceTrace );
	tracePtr.AppendNum( timeFrom1970, EHex );
			
    // Append call stack address count
    tracePtr.Append( KSpaceTrace );
    tracePtr.AppendNum( addrCount, EHex );
    
    // Packet number
    TUint packetNumber( 1 );
    
    // Calculate last item length
    TInt lastItemLength( KTraceMessage().Length() + KHexa32Length + 
            KSpaceLength + KNewlineLength );
    
    // Go through all call stack's memory addresses associated with
    // this memory allocation 
    for ( TInt j = 0; j < addrCount; j++ )
        {
        if ( tracePtr.Length() <= 0 )
            {
            // Memory deallocation fragment message.
            // FREEF <Memory address> <Time stamp> <Packet number> <Call stack address count>
            // <Call stack address>...
            // Create free fragment message header
            tracePtr.Append( KMemoryFreedFragment );
            tracePtr.AppendNum( memAddress, EHex );
            tracePtr.Append( KSpaceTrace );    
			tracePtr.AppendNum( timeFrom1970, EHex );
			tracePtr.Append( KSpaceTrace );	
            tracePtr.AppendNum( packetNumber, EHex );
            // Increase packet number
            packetNumber++;
            }
        
        // Read the next call stack's memory address stored in the buffer.
        stackBuf->Read( pos, &callStackAddr, KWordSize );
        
        // Append the read memory address as a hexadecimal number
        tracePtr.AppendFormat( KHexaNumberTrace, callStackAddr );
        
        // Move the pos variable one word onwards.
        pos += KWordSize;
        
        // Check if buffer max length exceed
        if ( lastItemLength + tracePtr.Length() >= KMemFreedBufLength )
            {
            tracePtr.Append( KNewLineTrace );
            // Log through debug channel 
            RDebug::Print( KTraceMessage, iProcessId, traceBuf );
            // Empty trace buffer
            tracePtr.Delete( 0, tracePtr.MaxLength() );
            }
        }
    
    // Send the last message if exists
    if ( tracePtr.Length() > 0 )
        {
        tracePtr.Append( KNewLineTrace );
        
        // Log through debug channel 
        RDebug::Print( KTraceMessage, iProcessId, traceBuf );
        }
        
    delete traceBuf;
    delete stackBuf;
    // If we are here, everything has gone alright
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CATStorageServerSession::LogProcessEndedL()
// Prints memory leaks and information on process end into a file opened by the
// function LogProcessStartedL()
// -----------------------------------------------------------------------------
//
TInt CATStorageServerSession::LogProcessEndedL( const RMessage2& aMessage )
    {
    LOGSTR1( "STSE TInt CATStorageServerSession::LogProcessEndedL()" );
    
    // Panic the client and return, if a logging session is not ongoing
    // ( can be started by calling the client's LogProcessStarted() )
    if ( !iLoggingOngoing )
        {
        PanicClient( EAToolNotAllowed, aMessage );
        return KErrCancel;
        }
    
    iError = KErrNone;
    
    // Read the sent process ID
    TUint processId = aMessage.Int0();
 
    // The process ID got from the client should equal iProcessId.
    // If it does not, return KErrNotSupported
    if ( processId != iProcessId )
        {
        return KErrNotSupported;
        }
    
    //////////////////////////////////////////////
    // Log memory leaks
    //////////////////////////////////////////////  
    
    // Print the information on the memory allocations that were never freed
    iError = PrintLeaksL( aMessage );
    
    if ( iError != KErrNone )
        {
        return iError;
        }    
    
    //////////////////////////////////////////////
    // Log handle leaks
    ////////////////////////////////////////////// 

    TUint handleLeakCount = aMessage.Int1();
    
    if( handleLeakCount == 0 )
        {
        LOGSTR1( "STSE TInt CATStorageServerSession::LogProcessEndedL() No handle leaks to report" );
        }
    else
        {
        // Make a buffer that will be logged
        TBuf8<KHandleLeakBufLength> loggingBuf;
        
        // Set handle leak module name to unknown since it can not be defined.
        // Write the handle leak count from aMessage.
        loggingBuf.Format( KHandleLeak, &KUnknownModule, handleLeakCount );
    
        // Write the constructed string into the data file and return if error
        iError = iFile.Write( loggingBuf );
    
        if ( iError != KErrNone )
            {
            return iError;
            }
        }
    
    //////////////////////////////////////////////
    // Log process end
    //////////////////////////////////////////////
           
    // Make a buffer that will be logged into the opened logging file
    TBufC8<KProcessEndBufLength> processEndBuf;
    
    TPtr8 bufPtr = processEndBuf.Des();
    
    bufPtr.AppendFormat( KProcessEnd, iProcessId );
    
    // Get the current universal time       
    TInt64 timeFrom1970( GetTime() );
    
    // Append the current time in the 64-bit (max 16 characters) hexadecimal text
    // format         
    bufPtr.AppendNum( timeFrom1970, EHex );
    
    // Append a new line
    bufPtr.Append( KNewLine );
    
    // Write the buffer into a file and return the error code   
    iError = iFile.Write( processEndBuf );
    
    // Close the file and the handle to the file server
    CloseFsAndFile();
    
    // Remove the process from the server's array of processes
    iError = iStorageServer.RemoveProcessL( processId );
    
    // Reset iProcesssId and set the logging flag false
    iProcessId = KNullProcessId;
    iLoggingOngoing = EFalse;
    
    return iError;
    }

// -----------------------------------------------------------------------------
// CATStorageServerSession::LogProcessEndTraceL()
// -----------------------------------------------------------------------------
//    
TInt CATStorageServerSession::LogProcessEndTraceL( const RMessage2& aMessage )
    {
    LOGSTR1( "STSE TInt CATStorageServerSession::LogProcessEndTraceL()" );
    
    // Panic the client and return, if a logging session is not ongoing
    // ( can be started by calling the client's LogProcessStarted() )
    if ( !iLoggingOngoing )
        {
        PanicClient( EAToolNotAllowed, aMessage );
        return KErrCancel;
        }
    
    iError = KErrNone;
    
    // Read the sent process ID
    TUint processId = aMessage.Int0();
 
    // The process ID got from the client should equal iProcessId.
    // If it does not, return KErrNotSupported
    if ( processId != iProcessId )
        {
        return KErrNotSupported;
        }
    
    //////////////////////////////////////////////
    // Log handle leaks
    ////////////////////////////////////////////// 

    TUint handleLeakCount = aMessage.Int1();

    if( handleLeakCount == 0 )
        {
        LOGSTR1( "STSE TInt CATStorageServerSession::LogProcessEndTraceL() No handle leaks to report" );
        }
    else
        {
        // Make a buffer that will be logged
        TBuf8<KHandleLeakBufLength> loggingBuf;
        
        // Make a 16-bit buffer that can be logged using RDebug
        TBuf<KHandleLeakBufLength> traceBuf;
        
        // Set handle leak module name to unknown since it can not be defined.
        // Write the handle leak count from aMessage.
        loggingBuf.Format( KHandleLeak, &KUnknownModule, handleLeakCount );
        
        traceBuf.Copy( loggingBuf );
    
        // Log through debug channel 
        RDebug::Print( KTraceMessage, iProcessId , &traceBuf );
        }

    //////////////////////////////////////////////
    // Log process end
    //////////////////////////////////////////////
    
    // Make a buffer that will be logged
    TBuf<KProcessEndBufLength> processEndBuf;    
    processEndBuf.AppendFormat( KProcessEndTrace, iProcessId );
    
    // Get the current universal time
    TInt64 timeFrom1970( GetTime() );
    
    // Append the current time in the 64-bit (max 16 characters) hexadecimal text
    // format
    processEndBuf.AppendNum( timeFrom1970, EHex );
    
    // Append a new line
    processEndBuf.Append( KNewLineTrace );
    
    // Log through debug channel
    RDebug::Print( KTraceMessage, iProcessId, &processEndBuf );

    // Remove the process from the server's array of processes
    iError = iStorageServer.RemoveProcessL( iProcessId );

    // Reset iProcesssId and set the logging flag false
    iProcessId = KNullProcessId;
    iLoggingOngoing = EFalse;
    
    return iError;
    }

// -----------------------------------------------------------------------------
// CATStorageServerSession::CheckMemoryAddressL()
// Checks if given memory address can be found
// -----------------------------------------------------------------------------
//
TInt CATStorageServerSession::CheckMemoryAddressL( const RMessage2& aMessage )
    {
    LOGSTR1( "STSE TInt CATStorageServerSession::CheckMemoryAddressL()" );
    
    // Panic the client and return, if a logging session is not ongoing
    // ( can be started by calling the client's LogProcessStarted() )
    if ( !iLoggingOngoing )
        {
        PanicClient( EAToolNotAllowed, aMessage );
        return KErrCancel;
        }
    
    iError = KErrNone;
    
    // Check if memory address can be found in iLeakArray
    TUint32 memAddress = aMessage.Int0();
    TIdentityRelation<CATMemoryEntry> matcher( CATMemoryEntry::Match );
    CATMemoryEntry* entry = new (ELeave) CATMemoryEntry( memAddress, NULL, 0, 0 );
    
    // Get the index or an error code
    iError = iLeakArray.Find( entry, matcher );
    delete entry;
    
    return iError;
    }

// -----------------------------------------------------------------------------
// CATStorageServerSession::CheckMemoryAddressTrace()
// Checks if some memory address can be found
// -----------------------------------------------------------------------------
//
TInt CATStorageServerSession::CheckMemoryAddressTrace( const RMessage2& aMessage )
    {
    LOGSTR1( "STSE TInt CATStorageServerSession::CheckMemoryAddressTrace()" );
    
    // Panic the client and return, if a logging session is not ongoing
    // ( can be started by calling the client's LogProcessStarted() )
    if ( !iLoggingOngoing )
        {
        PanicClient( EAToolNotAllowed, aMessage );
        return KErrCancel;
        }
    
    // Always return KErrNone in this mode
    return KErrNone;
    }
 
// -----------------------------------------------------------------------------
// CATStorageServerSession::GetProcessesL()
// Checks if some memory address can be found
// -----------------------------------------------------------------------------
//
TInt CATStorageServerSession::GetProcessesL( const RMessage2& aMessage )
    {
    LOGSTR1( "STSE TInt CATStorageServerSession::GetProcessesL()" );
    
    iError = KErrNone;
    
    TInt processInfoSize = sizeof( TATProcessInfo );
    
    CBufFlat* processInfoBuf;
    
    // Buffer position
    TInt pos( 0 );
    
    // Calculate the length of the buffer to be constructed for processes
    // One word will be reserved for the length of the array.
    TInt bufferLength = KWordSize + KATMaxProcesses * processInfoSize;
    
    // Construct processInfoBuf and expand it before the beginning (index 0)
    processInfoBuf = CBufFlat::NewL( bufferLength );
    CleanupStack::PushL( processInfoBuf );
    processInfoBuf->ExpandL( pos, bufferLength );
    
    RArray<TATProcessInfo> processArray = iStorageServer.ProcessInfoArray();

    // Variable for the number of TATProcessInfo objects in processArray
    TInt count = processArray.Count();
    
    // The count cannot be greater than KATMaxProcesses, because the client
    // has reserved a buffer of this size to be filled by the server
    if ( count > KATMaxProcesses )
        {
        count = KATMaxProcesses;
        }

    // Write the count (4 bytes) into the beginning of processInfoBuf
    processInfoBuf->Write( pos, &count, KWordSize );
    
    // Move the position one word onwards.    
    pos += KWordSize;
    
    // Write all the process info objects into the buffer 
    for ( TInt i = 0; i < count; i++ )
        {
        TATProcessInfo& processInfo = processArray[i];
        
        // Write the current process info into the buffer
        processInfoBuf->Write( pos, &processInfo, processInfoSize );
        
        // Move the pos variable onwards.
        pos += processInfoSize;
        }
    
    // Make a pointer descriptor pointing to the start of processInfoBuf
    TPtr8 bufPtr( processInfoBuf->Ptr(0) );
    
    // Write the buffer into aMessage at index 0 for the client
    aMessage.WriteL( 0, bufPtr );
    
    CleanupStack::PopAndDestroy( processInfoBuf );
    
    return iError;
    }

// -----------------------------------------------------------------------------
// CATStorageServerSession::GetDllsL()
// Checks if some memory address can be found
// -----------------------------------------------------------------------------
//
TInt CATStorageServerSession::GetDllsL( const RMessage2& aMessage )
    {
    LOGSTR1( "STSE TInt CATStorageServerSession::GetDllsL()" );
    
    // Read the process ID at index 0
    TUint processId = aMessage.Int0();
    
    // Size of a DLL descriptor
    TInt sizeOfDllDesc = sizeof( TBuf8<KMaxLibraryName> );
    
    // Buffer position
    TInt pos( 0 );
    
    // Calculate the length of the buffer to be constructed for DLL names.
    // One word will be reserved for the length of the array.
    TInt bufferLength = KWordSize + KATMaxDlls * sizeOfDllDesc;
    
    CBufFlat* dllBuf;
    // Construct dllBuf and expand it before the beginning (index 0)
    dllBuf = CBufFlat::NewL( bufferLength );
    CleanupStack::PushL( dllBuf );
    dllBuf->ExpandL( pos, bufferLength );

    RPointerArray<CATDynProcessInfo> dynProcessArray =
                                             iStorageServer.DynProcessInfoArray();
    
    // Construct a CATDynProcessInfo object with the given process ID for searching
    CATDynProcessInfo* dynProcessInfo = new (ELeave) CATDynProcessInfo( processId );
    
    // Find the index of a CATDynProcessInfo object with the given process ID
    TLinearOrder<CATDynProcessInfo> order( CATDynProcessInfo::Compare );
    TInt index = dynProcessArray.FindInOrder( dynProcessInfo, order );
    delete dynProcessInfo;
    dynProcessInfo = NULL;
    
    // Return, if a process with the requested process ID was not found 
    if ( index == KErrNotFound )
        {
        CleanupStack::PopAndDestroy( dllBuf );
        return index;
        }
    
    // Get the wanted dynamic process info 
    dynProcessInfo = dynProcessArray[index];
 
    // Fetch a reference to the desired DLL array
    RArray<TATDllInfo>& dllArray = dynProcessInfo->iDlls;
    
    // Take the count of names in the array 
    TInt count = dllArray.Count();

    // The count cannot be greater than KATMaxDlls, because the client
    // has reserved a buffer of this size to be filled by the server
    if ( count > KATMaxDlls )
        {
        count = KATMaxDlls;
        }

    // Write the count (4 bytes) into the beginning of dllBuf
    dllBuf->Write( pos, &count, KWordSize );
    
    // Move the position one word onwards.
    pos += KWordSize;

    // Go through all DLL names objects sent to the server 
    for ( TInt i = 0; i < count; i++ )
        {
        TBuf8<KMaxLibraryName>& dllName = dllArray[i].iName;
        
        // Write the current DLL name into the buffer
        dllBuf->Write( pos, &dllName, sizeOfDllDesc );
        
        // Move the pos variable onwards.
        pos += sizeOfDllDesc;
        }  
    
    // Make a pointer descriptor pointing to the start of dllBuf
    TPtr8 bufPtr( dllBuf->Ptr(0) );
    
    // Write the whole buffer into aMessage at index 1 for the client
    aMessage.WriteL( 1, bufPtr );
    
    CleanupStack::PopAndDestroy( dllBuf );

    // The dynProcessInfo object will not be deleted, because it is still owned by the
    // server object's dynamic process info array.  
    dynProcessInfo = NULL;
    
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CATStorageServerSession::GetLoggingModeL()
// -----------------------------------------------------------------------------
//    
TInt CATStorageServerSession::GetLoggingModeL( const RMessage2& aMessage )
    {
    LOGSTR1( "STSE TInt CATStorageServerSession::GetLoggingModeL()" );
    
    iError = KErrNone;
    
    // Read the process ID at index 0
    TUint processId = aMessage.Int0();
    
    // Buffer position
    TInt pos( 0 );
    
    // The length of the buffer to be constructed for logging mode
    TInt bufferLength = KWordSize;
    
    // Get the dynamic process info array
    RPointerArray<CATDynProcessInfo> dynProcessArray =
                                             iStorageServer.DynProcessInfoArray();
    
    // Construct a CATDynProcessInfo object with the given process ID for searching
    CATDynProcessInfo* dynProcessInfo = new (ELeave) CATDynProcessInfo( processId );
    
    // Find the index of a CATDynProcessInfo object with the given process ID
    TLinearOrder<CATDynProcessInfo> order( CATDynProcessInfo::Compare );
    TInt index = dynProcessArray.FindInOrder( dynProcessInfo, order );
    delete dynProcessInfo;
    dynProcessInfo = NULL;
    
    // Return, if a process with the requested process ID was not found 
    if ( index == KErrNotFound )
        {
        return index;
        }
    
    // Otherwise get the wanted dynamic process info
    dynProcessInfo = dynProcessArray[index];

    // Get the desired process's associated session object
    CATStorageServerSession* sessionObject = dynProcessInfo->iSessionObject;
    CBufFlat* loggingModeBuf;
    // Construct allocInfoBuf and expand it before the beginning (index 0)
    loggingModeBuf = CBufFlat::NewL( bufferLength );
    CleanupStack::PushL( loggingModeBuf );
    loggingModeBuf->ExpandL( 0, bufferLength );
    
    // Write the current logging mode of the requested process into the buffer.
    loggingModeBuf->Write( pos, &sessionObject->iLogOption, KWordSize );
    
    // Make a pointer descriptor that points to the data of allocInfoBuf
    TPtr8 bufPtr( loggingModeBuf->Ptr(0) );
    
    // Write the whole buffer into aMessage at index 1 for the client
    aMessage.WriteL( 1, bufPtr );
    
    CleanupStack::PopAndDestroy( loggingModeBuf );
    
    return iError;
    }

// -----------------------------------------------------------------------------
// CATStorageServerSession::StartSubtestL()
// -----------------------------------------------------------------------------
//    
TInt CATStorageServerSession::StartSubtestL( const RMessage2& aMessage )
    {
    LOGSTR1( "STSE TInt CATStorageServerSession::StartSubtestL()" );
    
    iError = KErrNone;
    
    // Read the process ID at index 0
    TUint processId = aMessage.Int0();
    
    // Read the sub test ID at index 1
    TBuf8<KATMaxSubtestIdLength> subTestName;
    iError = aMessage.Read( 1, subTestName );
    
    // Return if reading was not successful
    if ( iError != KErrNone )
        {
        return iError;
        }
    
    // Create another (non-8-bit) descriptor for logging to trace
    // and copy the contents
    TBuf<KATMaxSubtestIdLength> subTestNameTrace;
    subTestNameTrace.Copy( subTestName );

    // Read the handle count at index 2
    TInt handleCount = aMessage.Int2();
    
    // FIND THE REQUESTED PROCESS
    
    // Get the dynamic process array
    RPointerArray<CATDynProcessInfo> dynProcessArray =
                                             iStorageServer.DynProcessInfoArray();
    
    // Construct a CATDynProcessInfo object with the given process ID for searching
    CATDynProcessInfo* dynProcessInfo = new (ELeave) CATDynProcessInfo( processId );
    
    // Find the index of a CATDynProcessInfo object with the given process ID
    TLinearOrder<CATDynProcessInfo> order( CATDynProcessInfo::Compare );
    TInt index = dynProcessArray.FindInOrder( dynProcessInfo, order );
    delete dynProcessInfo;
    dynProcessInfo = NULL;
    
    // Return, if a process with the requested process ID was not found
    if ( index == KErrNotFound )
        {
        return index;
        }
    
    // Get the wanted dynamic process info
    dynProcessInfo = dynProcessArray[index];
    
    // Get the desired process's associated session object
    const CATStorageServerSession* sessionObject = dynProcessInfo->iSessionObject;
  
    // Make a buffer for logging thru trace
    TBuf<KTestStartBufLength> loggingBuf;
    
    // Copy the line tag into the buffer  
    loggingBuf.Copy( KSubtestStart );
    
    // Get the current universal time
    TInt64 timeFrom1970( GetTime() );
    
    // Append the current time in the 64-bit (max 16 characters) hexadecimal text
    // format
    loggingBuf.AppendNum( timeFrom1970, EHex );
    
    // Append a space
    loggingBuf.Append( KSpaceTrace );
    
    // Append the sub test ID
    loggingBuf.Append( subTestNameTrace );

    // Append a space
    loggingBuf.Append( KSpaceTrace );
    
    // Append current handle leak count
    loggingBuf.AppendNum( handleCount );
    
    // Append a new line
    loggingBuf.Append( KNewLineTrace );
    
    // Log the string through trace
    iError = sessionObject->LogThroughTrace( loggingBuf );
        
    // *******************
    // Send loaded DLL's
    // *******************
    
    // Fetch a reference to the desired DLL array
	RArray<TATDllInfo>& dllArray = dynProcessInfo->iDlls;
	
	// Take the count of dll info items
	TInt count( dllArray.Count() );
	LOGSTR2( "STSE > dllArray.Count( %i )", count );
	
	// Create buffers
	TBuf<KDllLoadBufLength> traceBuf;
	TBuf8<KDllLoadBufLength> dllBuf;
	
	for ( TInt x = 0; x < count; x++ )
		{
		dllBuf.Format( KDllLoad, &dllArray[x].iName, dllArray[x].iLoadTime,
				dllArray[x].iStartAddress, dllArray[x].iEndAddress );
		traceBuf.Copy( dllBuf );
		
		// Log the string through trace
		iError = sessionObject->LogThroughTrace( traceBuf );
		if ( iError != KErrNone )
			{
			LOGSTR2( "STSE > LogThroughTrace() err( %i )", iError );
			}
		}
    sessionObject = NULL;
    
    return iError;
    }

// -----------------------------------------------------------------------------
// CATStorageServerSession::StopSubtestL()
// -----------------------------------------------------------------------------
//
TInt CATStorageServerSession::StopSubtestL( const RMessage2& aMessage )
    {
    LOGSTR1( "STSE TInt CATStorageServerSession::StopSubtestL()" );
    
    iError = KErrNone;
    
    // Read the process ID at index 0
    TUint processId = aMessage.Int0();
    
    // Read the sub test ID at index 1
    TBuf8<KATMaxSubtestIdLength> subTestName;
    iError = aMessage.Read( 1, subTestName );
    
    // Return if reading was not successful
    if ( iError != KErrNone )
        {
        return iError;
        }

    // Create another (non-8-bit) descriptor for logging to trace,
    // and copy the contents
    TBuf<KATMaxSubtestIdLength> subTestNameTrace;
    subTestNameTrace.Copy( subTestName );

    // Read the handle count at index 2
    TInt handleCount = aMessage.Int2();
    
    // FIND THE REQUESTED PROCESS
    
    // Get the dynamic process array
    RPointerArray<CATDynProcessInfo> dynProcessArray =
                                             iStorageServer.DynProcessInfoArray();
    
    // Construct a CATDynProcessInfo object with the given process ID for searching
    CATDynProcessInfo* dynProcessInfo = new (ELeave) CATDynProcessInfo( processId );
    
    // Find the index of a CATDynProcessInfo object with the given process ID
    TLinearOrder<CATDynProcessInfo> order( CATDynProcessInfo::Compare );
    TInt index = dynProcessArray.FindInOrder( dynProcessInfo, order );
    delete dynProcessInfo;
    dynProcessInfo = NULL;
    
    // Return, if a process with the requested process ID was not found
    if ( index == KErrNotFound )
        {
        return index;
        }
    
    // Get the wanted dynamic process info
    dynProcessInfo = dynProcessArray[index];
    
    // Get the desired process's associated session object
    const CATStorageServerSession* sessionObject = dynProcessInfo->iSessionObject;
  
    // Make a buffer for logging thru trace
    TBuf<KTestEndBufLength> loggingBuf;
    
    // Copy the line tag into the buffer  
    loggingBuf.Copy( KSubtestEnd );
    
    // Get the current universal time
    TInt64 timeFrom1970( GetTime() );
    
    // Append the current time in the 64-bit (max 16 characters) hexadecimal text
    // format
    loggingBuf.AppendNum( timeFrom1970, EHex );
    
    // Append a space
    loggingBuf.Append( KSpaceTrace );
    
    // Append the sub test ID
    loggingBuf.Append( subTestNameTrace );

    // Append a space
    loggingBuf.Append( KSpaceTrace );
    
    // Append current handle leak count
    loggingBuf.AppendNum( handleCount );
    
    // Append a new line
    loggingBuf.Append( KNewLineTrace );
    
    // Log the string through trace
    iError = sessionObject->LogThroughTrace( loggingBuf );
    
    sessionObject = NULL;
    
    return iError;
    }

// -----------------------------------------------------------------------------
// CATStorageServerSession::StartSubtest2L()
// -----------------------------------------------------------------------------
//    
TInt CATStorageServerSession::StartSubtest2L( const RMessage2& aMessage )
    {
    LOGSTR1( "STSE TInt CATStorageServerSession::StartSubtest2L()" );
    
    iError = KErrNone;
    
    // Read the sub test ID at index 0
    TBuf8<KATMaxSubtestIdLength> subTestName;
    iError = aMessage.Read( 0, subTestName );
    
    // Return if reading was not successful
    if ( iError != KErrNone )
        {
        return iError;
        }
    
    // Create another (non-8-bit) descriptor for logging to trace
    // and copy the contents
    TBuf<KATMaxSubtestIdLength> subTestNameTrace;
    subTestNameTrace.Copy( subTestName );

    // Make a buffer for logging thru trace
    TBuf<KTestStartBufLength> loggingBuf;
    
    // Copy the line tag into the buffer  
    loggingBuf.Copy( KSubtestStart );
    
    // Get the current universal time
    TInt64 timeFrom1970( GetTime() );
    
    // Append the current time in the 64-bit (max 16 characters) hexadecimal text
    // format
    loggingBuf.AppendNum( timeFrom1970, EHex );
    
    // Append a space
    loggingBuf.Append( KSpaceTrace );
    
    // Append the sub test ID
    loggingBuf.Append( subTestNameTrace );
    
    // Append a new line
    loggingBuf.Append( KNewLineTrace );
    
    // Log the string through trace
    iError = LogThroughTrace( loggingBuf );
    
    // *******************
	// Send loaded DLL's
	// *******************
	
    // Get the dynamic process array
	RPointerArray<CATDynProcessInfo> dynProcessArray =
		iStorageServer.DynProcessInfoArray();
	
	// Construct a CATDynProcessInfo object with the given process ID for searching
	CATDynProcessInfo* dynProcessInfo = new (ELeave) CATDynProcessInfo( iProcessId );
	
	// Find the index of a CATDynProcessInfo object with the given process ID
	TLinearOrder<CATDynProcessInfo> order( CATDynProcessInfo::Compare );
	TInt index = dynProcessArray.FindInOrder( dynProcessInfo, order );
	delete dynProcessInfo;
	dynProcessInfo = NULL;
	 
	// Return, if a process with the requested process ID was not found
	if ( index == KErrNotFound )
		{
		return index;
		}
	
	// Get the wanted dynamic process info
	dynProcessInfo = dynProcessArray[index];
        
	// Fetch a reference to the desired DLL array
	RArray<TATDllInfo>& dllArray = dynProcessInfo->iDlls;
	
	// Take the count of dll info items
	TInt count( dllArray.Count() );
	LOGSTR2( "STSE > dllArray.Count( %i )", count );
	
	// Create buffers
	TBuf<KDllLoadBufLength> traceBuf;
	TBuf8<KDllLoadBufLength> dllBuf;
	
	for ( TInt x = 0; x < count; x++ )
		{
		dllBuf.Format( KDllLoad, &dllArray[x].iName, dllArray[x].iLoadTime,
				dllArray[x].iStartAddress, dllArray[x].iEndAddress );
		traceBuf.Copy( dllBuf );
		
		// Log the string through trace
		iError = LogThroughTrace( traceBuf );
		if ( iError != KErrNone )
			{
			LOGSTR2( "STSE > LogThroughTrace() err( %i )", iError );
			}
		}
    	
    return iError;
    }
    
// -----------------------------------------------------------------------------
// CATStorageServerSession::StopSubtest2()
// -----------------------------------------------------------------------------
//
TInt CATStorageServerSession::StopSubtest2( const RMessage2& aMessage )
    {
    LOGSTR1( "STSE TInt CATStorageServerSession::StopSubtest2()" );
    
    iError = KErrNone;
    
    // Read the sub test ID at index 0
    TBuf8<KATMaxSubtestIdLength> subTestName;
    iError = aMessage.Read( 0, subTestName );
    
    // Return if reading was not successful
    if ( iError != KErrNone )
        {
        return iError;
        }

    // Create another (non-8-bit) descriptor for logging to trace,
    // and copy the contents
    TBuf<KATMaxSubtestIdLength> subTestNameTrace;
    subTestNameTrace.Copy( subTestName );

    // Make a buffer for logging thru trace
    TBuf<KTestEndBufLength> loggingBuf;
    
    // Copy the line tag into the buffer  
    loggingBuf.Copy( KSubtestEnd );
    
    // Get the current universal time    
    TInt64 timeFrom1970( GetTime() );
    
    // Append the current time in the 64-bit (max 16 characters) hexadecimal text
    // format
    loggingBuf.AppendNum( timeFrom1970, EHex );
    
    // Append a space
    loggingBuf.Append( KSpaceTrace );
    
    // Append the sub test ID
    loggingBuf.Append( subTestNameTrace );
    
    // Append a new line
    loggingBuf.Append( KNewLineTrace );
    
    // Log the string through trace
    iError = LogThroughTrace( loggingBuf );
    
    return iError;
    }

// -----------------------------------------------------------------------------
// CATStorageServerSession::GetCurrentAllocsL()
// -----------------------------------------------------------------------------
//    
TInt CATStorageServerSession::GetCurrentAllocsL( const RMessage2& aMessage )
    {
    LOGSTR1( "STSE TInt CATStorageServerSession::GetCurrentAllocsL()" );
    
    iError = KErrNone;
    
    // Read the process ID at index 0
    TUint processId = aMessage.Int0();
    TUint32 allocNumber( 0 );
    TUint32 allocSize( 0 );
    
    // Buffer position
    TInt pos( 0 );
    
    // The length of the buffer to be constructed for allocation number and size
    TInt bufferLength = KWordSize + KWordSize;
    
    // Get the dynamic process info array
    RPointerArray<CATDynProcessInfo> dynProcessArray =
                                             iStorageServer.DynProcessInfoArray();
    
    // Construct a CATDynProcessInfo object with the given process ID for searching
    CATDynProcessInfo* dynProcessInfo = new (ELeave) CATDynProcessInfo( processId );
    
    // Find the index of a CATDynProcessInfo object with the given process ID
    TLinearOrder<CATDynProcessInfo> order( CATDynProcessInfo::Compare );
    TInt index = dynProcessArray.FindInOrder( dynProcessInfo, order );
    delete dynProcessInfo;
    dynProcessInfo = NULL;
    
    // Return, if a process with the requested process ID was not found
    if ( index == KErrNotFound )
        {
        return index;
        }
    
    // Otherwise get the wanted dynamic process info
    dynProcessInfo = dynProcessArray[index];

    // Get the desired process's associated session object
    CATStorageServerSession* sessionObject = dynProcessInfo->iSessionObject;
    
    // Get the alloc info array of that session object
    RArray<TAllocInfo> allocInfo = sessionObject->AllocInfoArray();
    
    // Get the values for current allocations number and size
    allocNumber = allocInfo.Count();
    
    // Calculate the total size of the current allocations
    for ( TUint32 i = 0; i < allocNumber; i++ )
        {
        allocSize += allocInfo[i].iAllocSize;
        }
    
    LOGSTR2( "STSE allocSize: %u", allocSize );
    LOGSTR2( "STSE iCurAllocSize: %u", iCurAllocSize );
    
    CBufFlat* allocInfoBuf;
    // Construct allocInfoBuf and expand it before the beginning (index 0)
    allocInfoBuf = CBufFlat::NewL( bufferLength );
    CleanupStack::PushL( allocInfoBuf );
    allocInfoBuf->ExpandL( 0, bufferLength );
    
    // Write the current number of allocations of the requested process into the buffer.
    allocInfoBuf->Write( pos, &allocNumber, KWordSize );
    
    // Move the position one word onwards
    pos += KWordSize;
    
    // Write the current total size of the allocations of the requested process into the
    // buffer.
    allocInfoBuf->Write( pos, &allocSize, KWordSize );
    
    // Make a pointer descriptor that points to the data of allocInfoBuf
    TPtr8 bufPtr( allocInfoBuf->Ptr(0) );
    
    // Write the whole buffer into aMessage at index 1 for the client
    aMessage.WriteL( 1, bufPtr );
    
    CleanupStack::PopAndDestroy( allocInfoBuf );
                    
    return iError;
    }

// -----------------------------------------------------------------------------
// CATStorageServerSession::GetMaxAllocsL()
// -----------------------------------------------------------------------------
//    
TInt CATStorageServerSession::GetMaxAllocsL( const RMessage2& aMessage )
    {
    LOGSTR1( "STSE TInt CATStorageServerSession::GetMaxAllocsL()" );
    
    iError = KErrNone;
    
    // Read the process ID at index 0
    TUint processId = aMessage.Int0();
    TUint32 allocNumber( 0 );
    TUint32 allocSize( 0 );
    
    // Buffer position
    TInt pos( 0 );
    
    // The length of the buffer to be constructed for allocation number and size
    TInt bufferLength = KWordSize + KWordSize;
    
    // Get the dynamic process info array
    RPointerArray<CATDynProcessInfo> dynProcessArray =
                                             iStorageServer.DynProcessInfoArray();
    
    // Construct a CATDynProcessInfo object with the given process ID for searching
    CATDynProcessInfo* dynProcessInfo = new (ELeave) CATDynProcessInfo( processId );
    
    // Find the index of a CATDynProcessInfo object with the given process ID
    TLinearOrder<CATDynProcessInfo> order( CATDynProcessInfo::Compare );
    TInt index = dynProcessArray.FindInOrder( dynProcessInfo, order );
    delete dynProcessInfo;
    dynProcessInfo = NULL;
    
    // Return, if a process with the requested process ID was not found 
    if ( index == KErrNotFound )
        {
        return index;
        }
    
    // Otherwise get the wanted dynamic process info 
    dynProcessInfo = dynProcessArray[index];

    // Get the desired process's associated session object
    CATStorageServerSession* sessionObject = dynProcessInfo->iSessionObject;
    
    // Get values for the maximum allocations number and size
    allocNumber = sessionObject->iMaxAllocs;
    allocSize = sessionObject->iMaxAllocSize;
    
    CBufFlat* allocInfoBuf;
    // Construct allocInfoBuf and expand it before the beginning (index 0)
    allocInfoBuf = CBufFlat::NewL( bufferLength );
    CleanupStack::PushL( allocInfoBuf );
    allocInfoBuf->ExpandL( 0, bufferLength );
    
    // Write the maximum number of allocations of the requested process into the buffer.
    allocInfoBuf->Write( pos, &allocNumber, KWordSize );
    
    // Move the position one word onwards
    pos += KWordSize;
    
    // Write the maximum total size of the allocations of the requested process into the
    // buffer.
    allocInfoBuf->Write( pos, &allocSize, KWordSize );
    
    // Make a pointer descriptor that points to the data of allocInfoBuf
    TPtr8 bufPtr( allocInfoBuf->Ptr(0) );
    
    // Write the whole buffer into aMessage at index 1 for the client
    aMessage.WriteL( 1, bufPtr );
    
    CleanupStack::PopAndDestroy( allocInfoBuf );
                    
    return iError;
    }
  
// -----------------------------------------------------------------------------
// CATStorageServerSession::CancelLoggingL()
// -----------------------------------------------------------------------------
//    
TInt CATStorageServerSession::CancelLoggingL( const RMessage2& aMessage )
    {
    LOGSTR1( "STSE TInt CATStorageServerSession::CancelLoggingL()" );
    
    iError = KErrNone;
    
    // Read the process ID at index 0
    TUint processId = aMessage.Int0();
        
    // FIND THE REQUESTED PROCESS
    
    // Get the dynamic process array
    RPointerArray<CATDynProcessInfo> dynProcessArray =
                                             iStorageServer.DynProcessInfoArray();
    
    // Construct a CATDynProcessInfo object with the given process ID for searching
    CATDynProcessInfo* dynProcessInfo = new (ELeave) CATDynProcessInfo( processId );
    
    // Find the index of a CATDynProcessInfo object with the given process ID
    TLinearOrder<CATDynProcessInfo> order( CATDynProcessInfo::Compare );
    TInt index = dynProcessArray.FindInOrder( dynProcessInfo, order );
    delete dynProcessInfo;
    dynProcessInfo = NULL;
     
    // Return, if a process with the requested process ID was not found
    if ( index == KErrNotFound )
        {
        return index;
        }
    
    // Otherwise get the wanted dynamic process info
    dynProcessInfo = dynProcessArray[index];
    
    // Get the session object of the requested process
    CATStorageServerSession* sessionObject = dynProcessInfo->iSessionObject;
      
    // Get the current universal time
    TInt64 timeFrom1970( GetTime() );
    
    // Make a buffer for logging "logging cancelled"
    TBuf8<KCancelBufLength> loggingBuf;
    
    // Copy the "logging cancelled" tag into the buffer with the current time 
    loggingBuf.AppendFormat( KLoggingCancelled, timeFrom1970 );
    
    // Log the buffer eather to a file or to debug channel depending on the current
    // logging mode
    
    if ( sessionObject->iLoggingOngoing && 
         sessionObject->iLogOption == EATLogToFile )
        {
        // Write the buffer into the file  
        sessionObject->iFile.Write( loggingBuf );
        }
    
    else if ( sessionObject->iLoggingOngoing &&
              sessionObject->iLogOption == EATLogToTrace )
        {
        // Make a buffer for logging to trace
        TBuf<KCancelBufLength> traceBuf;
        traceBuf.Copy( loggingBuf );
        
        // Write the buffer into the debug channel
        RDebug::Print( KTraceMessage, processId ,&traceBuf );
        }
    
    // Switch off logging of the requested process 
    sessionObject->iLogOption = EATLoggingOff;
    
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CATStorageServerSession::HandleError
// Internally used for handling error situations.
// -----------------------------------------------------------------------------
// 
void CATStorageServerSession::HandleError( TInt aError )
    {
    LOGSTR1( "STSE void CATStorageServerSession::HandleError()" );
      
    // Get the current universal time
    TInt64 timeFrom1970( GetTime() );
        
    // Make a buffer that will be logged into the opened logging file
    TBufC8<KErrOccuredBufLength> loggingBuf;
    
    TPtr8 bufPtr( loggingBuf.Des() );
     
    // Write the error code to the buffer  
    bufPtr.Format( KErrorOccured, aError );
       
    // Append the current time in the 64-bit (max 16 characters) hexadecimal text
    // format
    bufPtr.AppendNum( timeFrom1970, EHex );
    
    // Append a new line
    bufPtr.Append( KNewLine );
            
    // Write the buffer into a file (if possible in the current condition)
    iFile.Write( loggingBuf );
    }

// -----------------------------------------------------------------------------
// CATStorageServerSession::HandleErrorTrace()
// Internally used for handling error situations.
// -----------------------------------------------------------------------------
//    
void CATStorageServerSession::HandleErrorTrace( TInt aError )
    {
    LOGSTR1( "STSE void CATStorageServerSession::HandleErrorTrace()" );
    
     // Get the current universal time
    TInt64 timeFrom1970( GetTime() );
        
    // Make a buffer that will be logged
    TBuf<KErrOccuredBufLength> traceBuf;
     
    // Write the error code to the buffer  
    traceBuf.Format( KErrorOccuredTrace, aError );
       
    // Append the current time in the 64-bit (max 16 characters) hexadecimal text
    // format
    traceBuf.AppendNum( timeFrom1970, EHex );
    
    // Append a new line
    traceBuf.Append( KNewLineTrace );
                 
    RDebug::Print( KTraceMessage, iProcessId , &traceBuf );
    }

// -----------------------------------------------------------------------------
// CATStorageServerSession::OpenFsAndFile
// Internally used for opening a handle to the file server and a file
// -----------------------------------------------------------------------------
// 
TInt CATStorageServerSession::OpenFsAndFile( const TDesC& aFileName, 
    const TDesC8& aProcessName )
    {
    LOGSTR1( "STSE TInt CATStorageServerSession::OpenFsAndFile()" );
    
    // Connect file server, return if error occured
    iError = iFileServer.Connect();
    if ( iError )
        {
        iFileServer.Close();
        return iError;
        }

    // Open a file
    TBuf<KMaxFileName> fileNameBuf;    
    iError = TATDriveInfo::CreatePath( fileNameBuf, aFileName, iFileServer );
                        
    // Return, if an error occured, and it
    // is not KErrAlreadyExists
    if ( iError && iError != KErrAlreadyExists )
        {
        iFileServer.Close();
        return iError;
        }
    
    // Save the file name for this session
    CnvUtfConverter::ConvertFromUnicodeToUtf8( iLogFile, fileNameBuf );
        
    // Try to open file
    CheckIfFileAlreadyExist( fileNameBuf );
    
    // If a data file with the requested name already existed, and was opened
    // successfully, check the version of the file. If the line telling the version of
    // the file is not the expected, replace the file
    // If cannot open the file(error is KErrInUse), generate new filename and 
    // then try to create new file.
    if ( iError == KErrNone )
        {
        CheckFileVersion( fileNameBuf );
        }    
    else if ( iError == KErrInUse )
        {
        GenerateNewFileName( fileNameBuf, aProcessName );
        
        // Save the file name for this session
        CnvUtfConverter::ConvertFromUnicodeToUtf8( iLogFile, fileNameBuf );
        }
    LOGSTR2( "STSE > iError(%i)", iError );
        
    // If the file does not exist, create it. Write also the version number of
    // the file at the beginning of the new file
    if ( iError == KErrNotFound )
        {
        iError = iFile.Create( iFileServer, fileNameBuf, EFileWrite );
        
        if ( !iError )
            {
            iError = iFile.Write( KDataFileVersion );
            }
        }   
       
    if ( iError )
        {
        iFile.Close();
        iFileServer.Close();
        return iError;
        }
        
    // Seek the end of the file and set the current file position there
    TInt offset = 0;
    iError = iFile.Seek( ESeekEnd, offset );
    
    return iError;
    }

// -----------------------------------------------------------------------------
// CATStorageServerSession::GenerateNewFileName
// Called internally when need generate new file name.
// -----------------------------------------------------------------------------
//                     
void CATStorageServerSession::GenerateNewFileName( TDes& aFileName,
    const TDesC8& aProcessName )
    {    
    LOGSTR1( "STSE void CATStorageServerSession::GenerateNewFileName()" );
        
    // Extension
    TBuf<KExtensionLength> extension;

    // Parse file extension
    ParseExtension( aFileName, extension );

    // Try to find UID3 from current process name
    TInt uidErr( KErrBadName );
    TBuf<KMaxFileName> unicodeFile;

    // Converts text encoded using the Unicode transformation format UTF-8 
    // into the Unicode UCS-2 character set. 
    CnvUtfConverter::ConvertToUnicodeFromUtf8( unicodeFile, aProcessName );
    LOGSTR2( "STSE > unicodeFile(%S)", &unicodeFile );

    // Find square brackets
    TInt sPos( unicodeFile.Find( KOpenSquareBracket ) );
    TInt ePos( unicodeFile.Find( KCloseSquareBracket ) );
    LOGSTR3( "STSE > sPos(%i), ePos(%i)", sPos, ePos );
            
    if ( sPos != KErrNotFound && ePos != KErrNotFound )
        {
        TBuf<KProcessUidLength> processUid;
        TInt pEnd( ePos - sPos - KOpenSquareBracket().Length() );
        LOGSTR2( "STSE > pEnd(%i)", pEnd );
        
        // Copy UID value
        if ( pEnd > 0 )
            {
            processUid.Copy( unicodeFile.Mid( 
                    sPos + KOpenSquareBracket().Length(), pEnd ) );
            LOGSTR2( "STSE > processUid(%S)", &processUid );
            }
        
        if ( aFileName.Find( processUid ) == KErrNotFound )
            {
            // UID not exist, create new filename
            // Append uid to filename (<file name>_<uid>.<extension>)
            aFileName.Append( KUnderLine );
            aFileName.Append( processUid );
            aFileName.Append( extension );
            // Try to open file
            CheckIfFileAlreadyExist( aFileName );

            if ( iError == KErrNone )
                {
                uidErr = KErrNone;
                CheckFileVersion( aFileName );
                }
            }
        }

    if ( uidErr == KErrBadName && iError != KErrNotFound )
        {
        // Need re-create file name, add end off file _xx (xx=01, 02...)
        LOGSTR2( "STSE > Re-create file name, aFileName(%S)", &aFileName );
                
        // Parse file extension if exists.
        ParseExtension( aFileName, extension );
        
        // Temp file name
        TBuf<KMaxFileName> tempName;
        
        for ( TInt i = KNameIndexStart; i < KNameIndexEnd; i++ )
            {
            tempName.Delete( 0, tempName.MaxLength() );
            tempName.Format( KFormat, &aFileName, &KUnderLine, i, &extension );
            LOGSTR2( "STSE > tempName(%S)", &tempName );
            // Try to open file
            CheckIfFileAlreadyExist( tempName );
            
            if ( iError == KErrNone || iError == KErrNotFound )
                {
                aFileName.Copy( tempName );
                break;
                }
            }
        }
    }

// -----------------------------------------------------------------------------
// CATStorageServerSession::ParseExtension
// Method is used to parse file name extension.
// -----------------------------------------------------------------------------
//                     
void CATStorageServerSession::ParseExtension( 
    TDes& aFileName, TDes& aExtension )
    {    
    LOGSTR2( "STSE void CATStorageServerSession::ParseExtension(%S)", 
            &aFileName );

    // Parse current file name
    TParse parse;
    // Set up the TParse object 
    parse.Set( aFileName, NULL, NULL );

    // Tests whether an extension is present.
    if ( parse.ExtPresent() )
        {
        // Gets the extension
        aExtension.Copy( parse.Ext() );
        // Remove extension from file name
        TInt pos( aFileName.Find( aExtension ) );
        aFileName.Delete( pos, aFileName.Length() );
        LOGSTR3( "STSE > aFileName(%S), aExtension(%S)", 
                &aFileName, &aExtension );
        }
    }

// -----------------------------------------------------------------------------
// CATStorageServerSession::CheckIfFileAlreadyExist
// Method is used to check that file exists and is valid.
// -----------------------------------------------------------------------------
//                     
void CATStorageServerSession::CheckIfFileAlreadyExist( 
    const TDes& aFileName )
    {    
    LOGSTR2( "STSE void CATStorageServerSession::CheckIfFileAlreadyExist(%S)", 
            &aFileName );
    
    iError = iFile.Open( iFileServer, aFileName, EFileWrite );
    LOGSTR2( "STSE > iError(%i)", iError );
    }

// -----------------------------------------------------------------------------
// CATStorageServerSession::CheckFileVersion
// Method is used to check file version.
// -----------------------------------------------------------------------------
//                     
void CATStorageServerSession::CheckFileVersion( 
    const TDes& aFileName )
    {    
    LOGSTR2( "STSE void CATStorageServerSession::CheckFileVersion(%S)", 
            &aFileName );

    TBuf8<KVersionStringLength> versionString;

    // Read version information from the beginning of the file (offset 0)
    iFile.Read( 0, versionString, KVersionStringLength );

    // Delete the existing file, if the version string read from the file does not
    // match with KDataFileVersion.
    if ( versionString.Compare( KDataFileVersion ) != 0 )
        {
        // Close the existing, opened file, and delete it
        iFile.Close();
        iError = iFileServer.Delete( aFileName );
        
        // If the deletion was successful, set iError = KErrNotFound, so a new
        // file will be created in the next few lines 
        if ( iError == KErrNone )
            {
            iError = KErrNotFound;
            }
        }
    }

// -----------------------------------------------------------------------------
// CATStorageServerSession::CloseFsAndFile
// Internally used for closing a handle to the file server and a file
// -----------------------------------------------------------------------------
// 
void CATStorageServerSession::CloseFsAndFile()
    {
    LOGSTR1( "STSE void CATStorageServerSession::CloseFsAndFile()" );
    
    // Close the file    
    iFile.Close();
                 
    // Close the server session and return the error code   
    iFileServer.Close();
    }

// -----------------------------------------------------------------------------
// CATStorageServerSession::PrintLeaksL()
// Called internally when a process is closed. Prints possible memory leaks
// -----------------------------------------------------------------------------
//
TInt CATStorageServerSession::PrintLeaksL( const RMessage2& aMessage )
    {
    LOGSTR1( "STSE TInt CATStorageServerSession::PrintLeaksL()" );
    
    // Panic both the client and the server, if this method is called in a wrong
    // state (logging should be ongoing, and the system should be logging into a file,
    // not into debug channel)
    if ( !iLoggingOngoing || iLogOption != EATLogToFile )
        {
        PanicClient( EAToolInternalError, aMessage );
        StorageServerPanic( KCategoryServer, EAToolInternalError );
        }
   
    LOGMEM;

    // A pointer to a buffer of call stack's memory addresses
    CBufFlat* stackBuf = NULL;
    
    iError = KErrNone;
    
    TUint32 callStackAddr;
    
    // Number of leaks
    TInt leakCount = iLeakArray.Count();
    
    // Variable for the number of memory addresses in the call stack
    TInt addrCount( 0 );
    
    // Buffer position
    TInt pos( 0 );
                     
    // Go through all the leaks 
    for ( TInt i = 0; i < leakCount; i++ ) 
        {   
        pos = 0;
        
        // Get the call stack buffer of the the leak i.
        stackBuf = const_cast<CBufFlat*>( iLeakArray[i]->iCallstackBuf );
    
        // Read the first word of the buffer. This includes the number of
        // memory addresses stored in the current stackBuf
        stackBuf->Read( pos, &addrCount, KWordSize );
    
        // Move the position one word onwards.
        pos += KWordSize;
         
        // Construct a buffer for the string to be written into the logging file
        // because of this memory leak. MEM_LEAK: <Memory address> <Time stamp>
        // <Allocation size> <Call stack address> <Call stack address> ...
        HBufC8* leakString = 
            HBufC8::NewL( KMemleakLength +
                          KHexa32Length +
                          KSpaceLength + KHexa64Length +
                          KSpaceLength + KHexa32Length +
                          ( addrCount * (KSpaceLength + KHexa32Length) ) +
                          KNewlineLength 
                        );
            
        // Make a pointer descriptor that points to leakString
        TPtr8 leakStringPtr( leakString->Des() );
        
        // Append the tag implying a memory leak line in the data file
        leakStringPtr.Append( KMemoryLeak );
        
        // Append the address of the memory leak         
        TUint32 memAddress = iLeakArray[i]->iMemAddress;
        leakStringPtr.AppendNum( memAddress, EHex );
        
        // Append the current time in the 64-bit (max 16 characters) hexadecimal text
        // format
        leakStringPtr.Append( KSpace );
        TInt64 allocTime = iLeakArray[i]->iAllocTime;
        leakStringPtr.AppendNum( allocTime, EHex );
        
        // Append the size of the allocation in the 32-bit (max 8 characters) hexadecimal
        // text format.
        leakStringPtr.Append( KSpace );
        TInt allocSize = iLeakArray[i]->iAllocSize;
        leakStringPtr.AppendNum( allocSize, EHex );
        
        // Go through all call stack's memory addresses associated with
        // the current memory leak 
        for ( TInt j = 0; j < addrCount; j++ )
            {
            // Read the next call stack's memory address stored in the buffer.
            stackBuf->Read( pos, &callStackAddr, KWordSize );
            
            // Append the read memory address as a hexadecimal number
            leakStringPtr.AppendFormat( KHexaNumber,  callStackAddr );
    
            // Move the pos variable one word onwards.
            pos += KWordSize;
            }
        
        leakStringPtr.Append( KNewLine );
        
        // Set stackBuf to NULL, because it is not used anymore.
        stackBuf = NULL;
        
        // Write the constructed string into the data file and return if error
        iError = iFile.Write( *leakString );
        
        delete leakString;
          
        if ( iError != KErrNone )
            {
            return iError;
            }
              
        } // The outer for
   
    LOGSTR1( "STSE End of CATStorageServerSession::PrintLeaks()" );
    LOGMEM;
   
    // Empty the leak array and delete the referenced objects
    iLeakArray.ResetAndDestroy();
   
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CATStorageServerSession::SetLogOption()
// For setting the logging mode.
// -----------------------------------------------------------------------------
//
void CATStorageServerSession::SetLogOption( const RMessage2& aMessage )
    {
    LOGSTR1( "STSE void CATStorageServerSession::SetLogOption()" );
        
    // Panic both the client and the server, if this method is called in a wrong
    // state (logging must not be ongoing when changing the mode of operation).
    // So, the mode cannot be changed "on the fly".
    if ( iLoggingOngoing )
        {
        PanicClient( EAToolInternalError, aMessage );
        StorageServerPanic( KCategoryServer, EAToolInternalError );
        }    

    iLogOption = static_cast<TATLogOption>( aMessage.Int3() );
    
    // The default is EATLogToFile
    if ( iLogOption == EATUseDefault )
        {
        iLogOption = KDefaultLoggingMode;
        } 
    }
    
// -----------------------------------------------------------------------------
// CATStorageServerSession::LogThroughTrace()
// -----------------------------------------------------------------------------
//
TInt CATStorageServerSession::LogThroughTrace( const TDesC& aLogString ) const
    {
    LOGSTR1( "STSE TInt CATStorageServerSession::LogThroughTrace()" );

    // Return KErrNotSupported, if a logging session is not currently ongoing, or
    // the logging mode is not EATLogToTrace
    if ( !iLoggingOngoing || iLogOption != EATLogToTrace)
        {
        return KErrNotSupported;
        }
        
    RDebug::Print( KTraceMessage, iProcessId, &aLogString );
    
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CATStorageServerSession::AllocInfoArray
// -----------------------------------------------------------------------------
// 
RArray<TAllocInfo>& CATStorageServerSession::AllocInfoArray()
    {
    LOGSTR1( "STSE RArray<TAllocInfo>& CATStorageServerSession::AllocInfoArray()" );
     
    return iAllocInfoArray;
    }    

// -----------------------------------------------------------------------------
// CATStorageServerSession::PanicClient
// Creates a panic in the associated client's code.
// -----------------------------------------------------------------------------
//
void CATStorageServerSession::PanicClient( TInt aPanic, const RMessage2& aMessage )
    {
    LOGSTR1( "STSE void CATStorageServerSession::PanicClient()" );
    
    aMessage.Panic( KCategoryClient, aPanic );
    }

// -----------------------------------------------------------------------------
// CATStorageServerSession::GetLoggingFileL()
// -----------------------------------------------------------------------------
//    
TInt CATStorageServerSession::GetLoggingFileL( const RMessage2& aMessage )
    {
    LOGSTR1( "STSE TInt CATStorageServerSession::GetLoggingFileL()" );
    
    iError = KErrNone;
    
    // Read the process ID at index 0
    TUint processId = aMessage.Int0();
    
    // Get the dynamic process info array
    RPointerArray<CATDynProcessInfo> dynProcessArray =
                                             iStorageServer.DynProcessInfoArray();
    
    // Construct a CATDynProcessInfo object with the given process ID for searching
    CATDynProcessInfo* dynProcessInfo = new (ELeave) CATDynProcessInfo( processId );
    
    // Find the index of a CATDynProcessInfo object with the given process ID
    TLinearOrder<CATDynProcessInfo> order( CATDynProcessInfo::Compare );
    TInt index = dynProcessArray.FindInOrder( dynProcessInfo, order );
    delete dynProcessInfo;
    dynProcessInfo = NULL;
    
    // Return, if a process with the requested process ID was not found 
    if ( index == KErrNotFound )
        {
        return index;
        }
    
    // Otherwise get the wanted dynamic process info
    dynProcessInfo = dynProcessArray[index];

    // Get the desired process's associated session object
    CATStorageServerSession* sessionObject = dynProcessInfo->iSessionObject;
            
    // Write the whole buffer into aMessage at index 1 for the client
    aMessage.WriteL( 1, sessionObject->iLogFile );
                    
    return iError;
    }

// -----------------------------------------------------------------------------
// CATStorageServerSession::GetUdebL()
// -----------------------------------------------------------------------------
//    
TInt CATStorageServerSession::GetUdebL( const RMessage2& aMessage )
    {
    LOGSTR1( "STSE TInt CATStorageServerSession::GetUdebL()" );
    
    iError = KErrNone;
    
    // Read the process ID at index 0
    TUint processId = aMessage.Int0();
    
    // Get the dynamic process info array
    RPointerArray<CATDynProcessInfo> dynProcessArray =
                                             iStorageServer.DynProcessInfoArray();
    
    // Construct a CATDynProcessInfo object with the given process ID for searching
    CATDynProcessInfo* dynProcessInfo = new (ELeave) CATDynProcessInfo( processId );
    
    // Find the index of a CATDynProcessInfo object with the given process ID
    TLinearOrder<CATDynProcessInfo> order( CATDynProcessInfo::Compare );
    TInt index = dynProcessArray.FindInOrder( dynProcessInfo, order );
    delete dynProcessInfo;
    dynProcessInfo = NULL;
    
    // Return, if a process with the requested process ID was not found 
    if ( index == KErrNotFound )
        {
        return index;
        }
    
    // Otherwise get the wanted dynamic process info
    dynProcessInfo = dynProcessArray[index];

    // Get the desired process's associated session object
    CATStorageServerSession* sessionObject = dynProcessInfo->iSessionObject;
    
    TBuf8<KMaxVersionName> isUdeb;
    if ( sessionObject->iIsUdeb == 1 )
        {
        isUdeb.Copy( KUdeb() );
        }
    else if ( sessionObject->iIsUdeb == 0 )
        {
        isUdeb.Copy( KUrel() );
        }
    else
        {
        return KErrNotFound;
        }
    // Write the whole buffer into aMessage at index 1 for the client
    aMessage.WriteL( 1, isUdeb );
    
    return iError;
    }

// -----------------------------------------------------------------------------
// CATStorageServerSession::SetUdeb()
// -----------------------------------------------------------------------------
//    
void CATStorageServerSession::SetUdeb( const RMessage2& aMessage )
    {
    LOGSTR1( "STSE void CATStorageServerSession::SetUdeb()" );
 
    iIsUdeb = aMessage.Int0();
    }

// -----------------------------------------------------------------------------
// CATStorageServerSession::LogAbnormalEnd()
// -----------------------------------------------------------------------------
//    
void CATStorageServerSession::LogAbnormalEnd()
    {
    LOGSTR1( "STSE void CATStorageServerSession::LogAbnormalEnd()" );
    
    // Get the current universal time     
    TInt64 timeFrom1970( GetTime() );
        
    switch ( iLogOption )
        {
        case EATLogToTrace:
            {            
            // Make a buffer that will be logged
            TBuf<KEndAbnormalBufLength> traceBuf;
            
            // Write the process id to the buffer  
            traceBuf.Format( KProcessEndAbnormalTrace, iProcessId );
            
            // Append the current time in the 64-bit (max 16 characters) hexadecimal text
            // format         
            traceBuf.AppendNum( timeFrom1970, EHex );
                
            // Append a new line
            traceBuf.Append( KNewLineTrace );
            
            // Write the buffer into the debug channel
            RDebug::Print( KTraceMessage, iProcessId, &traceBuf );
            }
        break;
        
        case EATLogToFile:
            {            
            // Make a buffer that will be logged
            TBuf8<KEndAbnormalBufLength> loggingBuf;
            
            // Write the process id to the buffer  
            loggingBuf.Format( KProcessEndAbnormal, iProcessId );
            
            // Append the current time in the 64-bit (max 16 characters) hexadecimal text
            // format         
            loggingBuf.AppendNum( timeFrom1970, EHex );
                
            // Append a new line
            loggingBuf.Append( KNewLine );
            
            // Write the buffer into a file (if possible in the current condition)
            iFile.Write( loggingBuf );
            }
        break;
              
        default:
            break;
        }    
    }

// -----------------------------------------------------------------------------
// CATStorageServerSession::GetTime()
// Get the current universal time
// -----------------------------------------------------------------------------
//    
TInt64 CATStorageServerSession::GetTime()
    {
    LOGSTR1( "STSE void CATStorageServerSession::GetTime()" );
    
    // Get the current universal time
    iTime.UniversalTime();
        
    // Change the time format that tells the number of microseconds from January First,
    // 0 AD nominal Gregorian, into a format that tells the number of microseconds from
    // January First, 1970 AD nominal Gregorian. This is a more generic format and
    // can be directly exploited by the PC code parsing the data file that this
    // server generates.        
    return ( iTime.Int64() - iMicroSecondsAt1970 );        
    }

//  End of File
