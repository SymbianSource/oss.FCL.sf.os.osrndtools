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

#include "MemSpyEngineServer.h"

// System includes
#include <e32svr.h>

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


CMemSpyEngineServer::CMemSpyEngineServer( CMemSpyEngine& aEngine )
:   CServer2( EPriorityNormal ), iEngine( aEngine )
    {
    }


CMemSpyEngineServer::~CMemSpyEngineServer()
    {
    }


void CMemSpyEngineServer::ConstructL()
    {
    StartL( KMemSpyServerName );
    }


CMemSpyEngineServer* CMemSpyEngineServer::NewL( CMemSpyEngine& aEngine )
    {
    CMemSpyEngineServer* self = new(ELeave) CMemSpyEngineServer( aEngine );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }


CSession2* CMemSpyEngineServer::NewSessionL( const TVersion& aVersion, const RMessage2& aMessage ) const
    {
    if  ( aVersion.iMajor != KMemSpyClientServerVersion )
        {
        RDebug::Printf( "[MemSpy] CMemSpyEngineSession::NewSessionL() - BAD VERSION" );
        User::Leave( KErrNotSupported );
        }
    //
    CMemSpyEngineSession* session = CMemSpyEngineSession::NewL( iEngine, aMessage );
	return session;
    }























CMemSpyEngineSession::CMemSpyEngineSession( CMemSpyEngine& aEngine )
:   iEngine( aEngine )
    {
    }


CMemSpyEngineSession::~CMemSpyEngineSession()
    {
#ifdef _DEBUG
    TPtrC pThreadName( KNullDesC );
    if  ( iClientThreadName )
        {
        pThreadName.Set( *iClientThreadName );
        }

    RDebug::Print( _L("[MemSpy] CMemSpyEngineSession::~CMemSpyEngineSession() - DEAD SESSION - this: 0x%08x, id: %4d, name: %S"), this, iClientThreadId, iClientThreadName );
#endif

    delete iClientThreadName;
    }


void CMemSpyEngineSession::ConstructL( const RMessage2& aMessage )
    {
	RThread thread;
    const TInt error = aMessage.Client( thread );
    CleanupClosePushL( thread );

    TRACE( RDebug::Printf( "[MemSpy] CMemSpyEngineSession::ConstructL() - this: 0x%08x - opening client thread - err: %d", this, error ) );

    User::LeaveIfError( error );

    const TFullName threadName( thread.FullName() );
    iClientThreadName = threadName.AllocL();
    iClientThreadId = thread.Id();

    CleanupStack::PopAndDestroy( &thread );

    TRACE( RDebug::Print( _L("[MemSpy] CMemSpyEngineSession::ConstructL() - NEW SESSION - this: 0x%08x, id: %4d, client: %S"), this, iClientThreadId, iClientThreadName ) );
    }


CMemSpyEngineSession* CMemSpyEngineSession::NewL( CMemSpyEngine& aEngine, const RMessage2& aMessage )
    {
    CMemSpyEngineSession* self = new(ELeave) CMemSpyEngineSession( aEngine );
    CleanupStack::PushL( self );
    self->ConstructL( aMessage );
    CleanupStack::Pop( self );
    return self;
    }


void CMemSpyEngineSession::ServiceL( const RMessage2& aMessage )
    {
    TRACE( RDebug::Print( _L("[MemSpy] CMemSpyEngineSession::ServiceL() - START - this: 0x%08x, fn: 0x%08x, id: %4d, client: %S"), this, aMessage.Function(), iClientThreadId, iClientThreadName ) );

    TRAPD( error, DoServiceL( aMessage ) );
    if  ( error != KErrNone )
        {
        RDebug::Print( _L("[MemSpy] CMemSpyEngineSession::ServiceL() - SERVICE ERROR - this: 0x%08x, fn: %d, err: %d, client: %S"), this, aMessage.Function(), error, iClientThreadName );
        }
    aMessage.Complete( error );

    TRACE( RDebug::Print( _L("[MemSpy] CMemSpyEngineSession::ServiceL() - END - this: 0x%08x, fn: 0x%08x, id: %4d, client: %S"), this, aMessage.Function(), iClientThreadId, iClientThreadName ) );
	}


void CMemSpyEngineSession::DoServiceL( const RMessage2& aMessage )
    {
    TInt error = KErrNone;

    // Check function attributes
    const TInt function = aMessage.Function() & KMemSpyOpFlagsTypeMask;
    const TInt argSpec = aMessage.Function() & KMemSpyOpFlagsInclusionMask;
    const TBool byThreadId = ( argSpec == KMemSpyOpFlagsIncludesThreadId );
    const TBool byThreadName = ( argSpec == KMemSpyOpFlagsIncludesThreadName );

    TRACE( RDebug::Printf( "[MemSpy] CMemSpyEngineSession::DoServiceL() - START - unmodified function: 0x%08x, opCode: %d [TID: %d, TN: %d]", aMessage.Function(), function, byThreadId, byThreadName ) );

    // Check function is supported and argument combination is valid
    error = ValidateFunction( function, byThreadId, byThreadName );
    TRACE( RDebug::Printf( "[MemSpy] CMemSpyEngineSession::DoServiceL() - validation result: %d", error ) );
    
    // Process function request
    if  ( error == KErrNone )
        {
        if  ( byThreadId )
            {
            TRACE( RDebug::Printf( "[MemSpy] CMemSpyEngineSession::DoServiceL() - [TID] thread-specific..." ) );
            
            const TThreadId threadId( aMessage.Int0() );
            HandleThreadSpecificOpL( function, threadId );
            }
        else if ( byThreadName )
            {
            TRACE( RDebug::Printf( "[MemSpy] CMemSpyEngineSession::DoServiceL() - [TN] thread-specific..." ) );

            error = aMessage.GetDesLength( 0 /*slot 0*/ );
        
            if  ( error > 0 && error <= KMaxFullName )
                {
                TFullName* threadName = new(ELeave) TFullName();
                CleanupStack::PushL( threadName );
                aMessage.ReadL( 0, *threadName );
                HandleThreadSpecificOpL( function, *threadName );
                CleanupStack::PopAndDestroy( threadName );
                }
            else
                {
                error = KErrArgument;
                }
            }
        else
            {
            TRACE( RDebug::Printf( "[MemSpy] CMemSpyEngineSession::DoServiceL() - thread-agnostic..." ) );

            HandleThreadAgnosticOpL( function, aMessage );
            }
        }

    User::LeaveIfError( error );

    TRACE( RDebug::Printf( "[MemSpy] CMemSpyEngineSession::DoServiceL() - END" ) );
    }



TInt CMemSpyEngineSession::ValidateFunction( TInt aFunction, TBool aIncludesThreadId, TBool aIncludesThreadName )
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
        RDebug::Printf( "[MemSpy] CMemSpyEngineSession::ValidateFunction() - function request did not validate - [withId: %d, withName: %d]", aIncludesThreadId, aIncludesThreadName );
        }
    //
    return err;
    }


void CMemSpyEngineSession::HandleThreadSpecificOpL( TInt aFunction, const TThreadId& aThreadId )
    {
    TRACE( RDebug::Printf( "[MemSpy] CMemSpyEngineSession::HandleThreadSpecificOpL() - START - aFunction: %d, aThreadId: %d", aFunction, (TUint) aThreadId ) );

    ASSERT( (TUint) aThreadId != 0 );
    TInt error = KErrNone;

    // Check if its a kernel thread identifier
    const TBool isKernel = ( static_cast<TUint32>( aThreadId ) == KMemSpyClientServerThreadIdKernel );

    // Treat as thread specific operation
    CMemSpyProcess* process = NULL;
    CMemSpyThread* thread = NULL;
    if  ( !isKernel )
        {
        error = iEngine.Container().ProcessAndThreadByThreadId( aThreadId, process, thread );
        TRACE( RDebug::Printf( "[MemSpy] CMemSpyEngineSession::HandleThreadSpecificOpL() - search result: %d, proc: 0x%08x, thread: 0x%08x", error, process, thread ) );
        }
    else
        {
        // Kernel is only supported for a couple of operations
        if  ( aFunction == EMemSpyClientServerOpHeapInfo || aFunction == EMemSpyClientServerOpHeapData )
            {
            }
        else
            {
            TRACE( RDebug::Printf( "[MemSpy] CMemSpyEngineSession::HandleThreadSpecificOpL() - trying to call unsupported function for kernel thread!" ) );
            error = KErrArgument;
            }
        }

    // Must be no error so far and we must have a valid thread & process when performing a non-kernel op
    // or then if we are performing a kernel op, we don't need the thread or process.
    if  ( error == KErrNone && ( ( thread && process && !isKernel ) || ( isKernel ) ) )
        {
#ifdef _DEBUG
        if  ( thread )
            {
            HBufC* threadName = thread->FullName().AllocLC();
            _LIT( KTrace2, "[MemSpy] CMemSpyEngineSession::HandleThreadSpecificOpL() - thread: %S" );
            RDebug::Print( KTrace2, threadName );
            CleanupStack::PopAndDestroy( threadName );
            }
        else if ( isKernel )
            {
            _LIT( KTrace2, "[MemSpy] CMemSpyEngineSession::HandleThreadSpecificOpL() - thread: Kernel" );
            RDebug::Print( KTrace2 );
            }
#endif

        // Got a valid thread object - now work out which operation to perform...
        switch( aFunction )
            {
        case EMemSpyClientServerOpSummaryInfo:
            iEngine.HelperProcess().OutputProcessInfoL( *process );
            break;
        case EMemSpyClientServerOpSummaryInfoDetailed:
            iEngine.HelperProcess().OutputProcessInfoDetailedL( *process );
            break;
        case EMemSpyClientServerOpHeapInfo:
            if  ( isKernel )
                {
                iEngine.HelperHeap().OutputHeapInfoKernelL();
                }
            else
                {
                iEngine.HelperHeap().OutputHeapInfoUserL( *thread );
                }
            break;
        case EMemSpyClientServerOpHeapCellListing:
            iEngine.HelperHeap().OutputCellListingUserL( *thread );
            break;
        case EMemSpyClientServerOpHeapData:
            if  ( isKernel )
                {
                iEngine.HelperHeap().OutputHeapDataKernelL();
                }
            else
                {
                iEngine.HelperHeap().OutputHeapDataUserL( *thread );
                }
            break;
        case EMemSpyClientServerOpStackInfo:
            iEngine.HelperStack().OutputStackInfoL( *thread );
            break;
        case EMemSpyClientServerOpStackDataUser:
            iEngine.HelperStack().OutputStackDataL( *thread, EMemSpyDriverDomainUser, EFalse );
            break;
        case EMemSpyClientServerOpStackDataKernel:
            iEngine.HelperStack().OutputStackDataL( *thread, EMemSpyDriverDomainKernel, EFalse );
            break;
        case EMemSpyClientServerOpOpenFiles:
            iEngine.HelperFileSystem().ListOpenFilesL( aThreadId );
            break;

        default:
            error = KErrNotSupported;
            break;
            }
        }

    TRACE( RDebug::Printf("[MemSpy] CMemSpyEngineSession::HandleThreadSpecificOpL() - END - aFunction: %d, aThreadId: %d, error: %d", aFunction, (TUint) aThreadId, error ) );
    User::LeaveIfError( error );
    }


void CMemSpyEngineSession::HandleThreadSpecificOpL( TInt aFunction, const TDesC& aThreadName )
    {
    TRACE( RDebug::Print( _L("[MemSpy] CMemSpyEngineSession::HandleThreadSpecificOpL() - START - aFunction: %d, aThreadName: %S"), aFunction, &aThreadName ) );
    //
    CMemSpyProcess* process = NULL;
    CMemSpyThread* thread = NULL;
    TInt error = iEngine.Container().ProcessAndThreadByPartialName( aThreadName, process, thread );
    User::LeaveIfError( error );
    //
    const TThreadId threadId( thread->Id() );
    HandleThreadSpecificOpL( aFunction, threadId );
    //
    TRACE( RDebug::Print( _L("[MemSpy] CMemSpyEngineSession::HandleThreadSpecificOpL() - END - aFunction: %d, aThreadName: %S"), aFunction, &aThreadName ) );
    }


void CMemSpyEngineSession::HandleThreadAgnosticOpL( TInt aFunction, const RMessage2& aMessage )
    {
    TRACE( RDebug::Printf("[MemSpy] CMemSpyEngineSession::HandleThreadAgnosticOpL() - START" ) );
    //
    if  ( aFunction ==  EMemSpyClientServerOpHeapInfoCompact )
        {
        TRACE( RDebug::Printf("[MemSpy] CMemSpyEngineSession::HandleThreadAgnosticOpL() - EMemSpyClientServerOpHeapInfoCompact") );
        iEngine.HelperHeap().OutputHeapInfoForDeviceL();
        }
    else if ( aFunction ==  EMemSpyClientServerOpStackInfoCompact )
        {
        TRACE( RDebug::Printf("[MemSpy] CMemSpyEngineSession::HandleThreadAgnosticOpL() - EMemSpyClientServerOpStackInfoCompact") );
        iEngine.HelperStack().OutputStackInfoForDeviceL();
        }
    else if ( aFunction == EMemSpyClientServerOpSystemWideMemoryTrackingTimerStart )
        {
        TRACE( RDebug::Printf("[MemSpy] CMemSpyEngineSession::HandleThreadAgnosticOpL() - EMemSpyClientServerOpSystemWideMemoryTrackingTimerStart") );
        iEngine.HelperSysMemTracker().StartL();
        }
    else if ( aFunction == EMemSpyClientServerOpSystemWideMemoryTrackingTimerStop )
        {
        TRACE( RDebug::Printf("[MemSpy] CMemSpyEngineSession::HandleThreadAgnosticOpL() - EMemSpyClientServerOpSystemWideMemoryTrackingTimerStop") );
        iEngine.HelperSysMemTracker().StopL();
        }
    else if ( aFunction == EMemSpyClientServerOpSystemWideMemoryTrackingReset )
        {
        TRACE( RDebug::Printf("[MemSpy] CMemSpyEngineSession::HandleThreadAgnosticOpL() - EMemSpyClientServerOpSystemWideMemoryTrackingReset") );
        iEngine.HelperSysMemTracker().Reset();
        }
    else if ( aFunction == EMemSpyClientServerOpSystemWideMemoryTrackingForceUpdate )
        {
        TRACE( RDebug::Printf("[MemSpy] CMemSpyEngineSession::HandleThreadAgnosticOpL() - EMemSpyClientServerOpSystemWideMemoryTrackingForceUpdate") );
        iEngine.HelperSysMemTracker().CheckForChangesNowL();
        }
    else if ( aFunction == EMemSpyClientServerOpSystemWideMemoryTrackingTimerPeriodSet )
        {
        TRACE( RDebug::Printf("[MemSpy] CMemSpyEngineSession::HandleThreadAgnosticOpL() - EMemSpyClientServerOpSystemWideMemoryTrackingTimerPeriodSet") );
        
        // Get current config
        TMemSpyEngineHelperSysMemTrackerConfig config;
        iEngine.HelperSysMemTracker().GetConfig( config );

        // Set new timer value
        config.iTimerPeriod = aMessage.Int0();

        // And update config... which will leave if the config is invalid
        iEngine.HelperSysMemTracker().SetConfigL( config );
        }
    else if ( aFunction == EMemSpyClientServerOpSystemWideMemoryTrackingCategoriesSet )
        {
        TRACE( RDebug::Printf("[MemSpy] CMemSpyEngineSession::HandleThreadAgnosticOpL() - EMemSpyClientServerOpSystemWideMemoryTrackingCategoriesSet") );
        // Get current config
        TMemSpyEngineHelperSysMemTrackerConfig config;
        iEngine.HelperSysMemTracker().GetConfig( config );

        // Set new categories
        config.iEnabledCategories = aMessage.Int0();

        // And update config... which will leave if the config is invalid
        iEngine.HelperSysMemTracker().SetConfigL( config );
        }
    else if ( aFunction == EMemSpyClientServerOpSystemWideMemoryTrackingThreadNameFilterSet )
        {
        TRACE( RDebug::Printf("[MemSpy] CMemSpyEngineSession::HandleThreadAgnosticOpL() - EMemSpyClientServerOpSystemWideMemoryTrackingThreadNameFilterSet") );
        // Get current config
        TMemSpyEngineHelperSysMemTrackerConfig config;
        iEngine.HelperSysMemTracker().GetConfig( config );

        // Set new filter
        RBuf buf;
        buf.CleanupClosePushL();
        TInt len = aMessage.GetDesLength( 0 );
        if ( len > 0 )
            {
            buf.CreateL( len );
            aMessage.ReadL( 0, buf, 0 );
            config.iThreadNameFilter.Copy( buf );            
            }
        else
            {
            config.iThreadNameFilter.Zero();
            }
        CleanupStack::PopAndDestroy( &buf );

        // And update config... which will leave if the config is invalid
        iEngine.HelperSysMemTracker().SetConfigL( config );
        }
    else if ( aFunction == EMemSpyClientServerOpSystemWideMemoryTrackingHeapDumpSet )
        {
        // Get current config
        TMemSpyEngineHelperSysMemTrackerConfig config;
        iEngine.HelperSysMemTracker().GetConfig( config );
        
        // Set new Heap Dump value
        config.iDumpData = aMessage.Int0();
        
        // And update config... which will leave if the config is invalid
        iEngine.HelperSysMemTracker().SetConfigL( config );
        }
    else if ( aFunction == EMemSpyClientServerOpSwitchOutputSinkTrace )
        {
        TRACE( RDebug::Printf("[MemSpy] CMemSpyEngineSession::HandleThreadAgnosticOpL() - EMemSpyClientServerOpSwitchOutputSinkTrace") );
        iEngine.InstallSinkL( ESinkTypeDebug );
        }
    else if ( aFunction == EMemSpyClientServerOpSwitchOutputSinkFile )
        {
        TRACE( RDebug::Printf("[MemSpy] CMemSpyEngineSession::HandleThreadAgnosticOpL() - EMemSpyClientServerOpSwitchOutputSinkFile") );
        iEngine.InstallSinkL( ESinkTypeFile );
        }
    else if ( aFunction == EMemSpyClientServerOpEnumerateKernelContainer )
        {
        const TMemSpyDriverContainerType type = CMemSpyEngineHelperKernelContainers::MapToType( static_cast< TObjectType >( aMessage.Int0() ) );

        TRACE( RDebug::Printf("[MemSpy] CMemSpyEngineSession::HandleThreadAgnosticOpL() - EMemSpyClientServerOpEnumerateKernelContainer - type: %d", type ) );

        CMemSpyEngineGenericKernelObjectList* model = iEngine.HelperKernelContainers().ObjectsForSpecificContainerL( type );
        CleanupStack::PushL( model );
        model->OutputL( iEngine.Sink() );
        CleanupStack::PopAndDestroy( model );
        }
    else if ( aFunction == EMemSpyClientServerOpEnumerateKernelContainerAll )
        {
        TRACE( RDebug::Printf("[MemSpy] CMemSpyEngineSession::HandleThreadAgnosticOpL() - EMemSpyClientServerOpEnumerateKernelContainerAll") );
        CMemSpyEngineGenericKernelObjectContainer* model = iEngine.HelperKernelContainers().ObjectsAllL();
        CleanupStack::PushL( model );
        model->OutputL( iEngine.Sink() );
        CleanupStack::PopAndDestroy( model );
        }
    else if ( aFunction == EMemSpyClientServerOpOpenFiles )
        {
        TRACE( RDebug::Printf("[MemSpy] CMemSpyEngineSession::HandleThreadAgnosticOpL() - EMemSpyClientServerOpOpenFiles") );
        iEngine.ListOpenFilesL();
        }
    else if ( aFunction == EMemSpyClientServerOpDisableAknIconCache )
        {
        TRACE( RDebug::Printf("[MemSpy] CMemSpyEngineSession::HandleThreadAgnosticOpL() - EMemSpyClientServerOpDisableAknIconCache") );
        iEngine.HelperRAM().SetAknIconCacheStatusL( EFalse );
        }
    else
        {
        TRACE( RDebug::Printf("[MemSpy] CMemSpyEngineSession::HandleThreadAgnosticOpL() - [device-wide operation] => invoking UI") );
        iEngine.NotifyClientServerOperationRequestL( aFunction );
        }
    //
    TRACE( RDebug::Printf("[MemSpy] CMemSpyEngineSession::HandleThreadAgnosticOpL() - END" ) );
    }





