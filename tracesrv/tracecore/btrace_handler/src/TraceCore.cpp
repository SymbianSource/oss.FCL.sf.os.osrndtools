// Copyright (c) 2007-2010 Nokia Corporation and/or its subsidiary(-ies).
// All rights reserved.
// This component and the accompanying materials are made available
// under the terms of "Eclipse Public License v1.0"
// which accompanies this distribution, and is available
// at the URL "http://www.eclipse.org/legal/epl-v10.html".
//
// Initial Contributors:
// Nokia Corporation - initial contribution.
//
// Contributors:
//
// Description:
//  Trace Core main instance implementation
//
//

#include <e32def.h>
#include <e32btrace.h>
#include <trace_certification.h>

#include "TraceCore.h"
#include "TraceCoreSendReceive.h"
#include "TraceCoreRouter.h"
#include "TraceCoreDebug.h"
#include "TraceCoreBTraceHandler.h"
#include "TraceCorePrintfTraceHandler.h"
#include "TraceCoreNotifier.h"
#include "TraceCoreConfiguration.h"
#include "TraceCoreInternalMessageHandler.h"
#include "TraceCoreActivation.h"

#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "TraceCoreTraces.h"
#endif

/**
 * TraceCore global instance
 */
DTraceCore* DTraceCore::iInstance = NULL;


/**
 * Gets the TraceCore instance,
 * @return DTraceCore object or NULL if TraceCore is not created yet.
 */
EXPORT_C DTraceCore* DTraceCore::GetInstance()
    {
    return iInstance;
    }

/**
 * Returns ETrue if tracecore is loaded already EFalse otherwies
 */
EXPORT_C TBool DTraceCore::IsLoaded()
    {
    return ( iInstance != NULL );
    }

/**
 * Gets the BTrace handler
 */
DTraceCoreBTraceHandler* DTraceCore::GetBTraceHandler()
    {
    DTraceCoreBTraceHandler* retval = NULL;
    DTraceCore* traceCore = DTraceCore::GetInstance();
    // BTrace handler is the first one to be registered
    if ( traceCore != NULL && traceCore->iHandlers.Count() > 0 )
        {
        retval = ( DTraceCoreBTraceHandler* )traceCore->iHandlers[ 0 ];
        }
    return retval;
    }

/**
 * Constructor
 */
DTraceCore::DTraceCore()
: iRouter( NULL )
, iSendReceive( NULL )
, iNotifier( NULL )
, iConfiguration( NULL )
, iInternalMessageHandler( NULL )
, iBTrace( NULL )
, iPrintf( NULL )
, iTraceCoreSettings( NULL )
, iActiveWriter( NULL )
, iActivationQ(NULL)
, iPreviousTraceDropped(EFalse)
, iTraceCertified(EFalse)
    {
    }

/**
 * Destructor
 */
DTraceCore::~DTraceCore()
    {
    // Writers and handlers are deleted first
    //  -> They call unregister functions
    delete iBTrace;
    delete iPrintf;

    iHandlers.Reset();
    iWriters.Reset();
    iActivations.Reset();

    delete iRouter;
    delete iSendReceive;
    delete iNotifier;
    delete iConfiguration;
	//destroy iActivationQue
    if (iActivationQ)
        iActivationQ->Destroy();
    }


/*
 * Init TraceCore
 */
TInt DTraceCore::Init()
    {
    TC_TRACE( ETraceLevelFlow, Kern::Printf( ">DTraceCore::Init()" ) );
    TInt ret;

    //TODO: measure how much time it takes to boot up
    //TODO: measure how much memory is consumed by TraceCore
   //TODO: check granularity of internal RArrays - update this for most used use case

    //TODO: check dfc que (if we have any) that deals with pulling trace data from buffer

   // Store the trace certified flag, used to suppress tracing on production images.
   // This stored value is used throughout the session, even if the status of the certificate changes.
   // This is done to avoid performance issues when performing the check with every trace outputted.
   iTraceCertified = TraceCertification::IsTraceCertified();

    //create activation/deactivation queue
   const TInt KTCDfcThreadPriority = 24; // Kern::DfcQ0 is 27
   //TODO: put this const to a header
   //TODO: check if I can use constants for this (DFCQ0-1) ?
   _LIT(KTCDfcQueue, "TraceCoreActivationDfcQ");
   Kern::Printf("Creating TC dfc - %S...", &KTCDfcQueue);
   ret = Kern::DynamicDfcQCreate(iActivationQ, KTCDfcThreadPriority, KTCDfcQueue);
   Kern::Printf("returned code: %d, ptr: 0x%x", ret, iActivationQ);
   if (ret==KErrNone)
       //disable real-time state of the dfcq
       iActivationQ->SetRealtimeState(ERealtimeStateOff);

    // Disable Component ID filtering in BTrace
    BTrace::SetFilter2(1);

    // Create needed components
    iRouter = new DTraceCoreRouter();
    iSendReceive = new DTraceCoreSendReceive();
    iNotifier = new DTraceCoreNotifier();
    iConfiguration = new DTraceCoreConfiguration();
    iInternalMessageHandler = new DTraceCoreInternalMessageHandler();

    // Check that creating of the components was succesfull
    if ( iRouter != NULL && iSendReceive != NULL && iNotifier != NULL && iConfiguration != NULL )
        {
        ret = iSendReceive->Init( *iRouter );
        if ( ret == KErrNone )
            {
            ret = iRouter->Init( *iSendReceive );
            if ( ret == KErrNone )
                {
                ret = iConfiguration->Init();
                if ( ret == KErrNone )
                    {
                    ret = iInternalMessageHandler->Init();
                    }
                }
            }
        }
    else
        {
        ret = KErrNoMemory;
        }

    // BTrace handler is integrated to TraceCore
    if ( ret == KErrNone )
        {
        ret = StartBTrace();
        }

    // Printf handler is integrated to TraceCore
    if ( ret == KErrNone )
        {
        ret = StartPrintfTraceHandler();
        }
    TC_TRACE( ETraceLevelFlow, Kern::Printf( "<DTraceCore::Init() - %d", ret ) );

    return ret;
    }

/**
 * Starts the BTrace handler
 */
TInt DTraceCore::StartBTrace()
    {
    TInt ret = KErrNone;

    // Create BTrace handler
    iBTrace = new DTraceCoreBTraceHandler();
    if ( iBTrace != NULL )
        {
        // Init calls RegisterHandler
        ret = iBTrace->Init();
        }
    else
        {
        ret = KErrNoMemory;
        }
    TC_TRACE( ETraceLevelFlow, Kern::Printf( "<DTraceCore::StartBTrace() - %d", ret ) );
    return ret;
    }


/**
 * Starts the Printf trace handler
 */
TInt DTraceCore::StartPrintfTraceHandler()
    {
    TInt ret;

    // Create PrintF handler
    iPrintf = new DTraceCorePrintfTraceHandler();
    if ( iPrintf != NULL )
        {
        // Init calls RegisterHandler
        ret = iPrintf->Init();
        }
    else
        {
        ret = KErrNoMemory;
        }
    TC_TRACE( ETraceLevelFlow, Kern::Printf( "<DTraceCore::StartPrintfTraceHandler() - %d", ret ) );
    return ret;
    }


/**
 * Registers an activation interface
 */
TInt DTraceCore::RegisterActivation( MTraceCoreActivation& aActivation )
    {
    return iActivations.Append( &aActivation );
    }


/**
 * Gets an activation interface.
 * Tracing from this method is not allowed.
 */
MTraceCoreActivation* DTraceCore::GetActivation( TUint32 aComponentId )
    {
    MTraceCoreActivation* retval = NULL;

    // Find the correct activation interface from the list
    for ( int i = 0; i < iActivations.Count(); i++ )
        {
        if ( iActivations[ i ]->IsComponentSupported( aComponentId ) )
            {
            retval = iActivations[ i ];
            }
        }

    return retval;
    }


/**
 * Registers a handler
 *
 * @param aHandler The handler
 */
TInt DTraceCore::RegisterHandler( DTraceCoreHandler& aHandler )
    {
    OstTrace1( TRACE_FLOW, DTRACECORE_REGISTERHANDLER_ENTRY, "> DTraceCore::RegisterHandler 0x%x", ( TUint )&( aHandler ) );
    TInt ret = iHandlers.Append( &aHandler );

    // If there is not yet active writer, set the new one
    if ( ret == KErrNone && iActiveWriter != NULL )
        {
        aHandler.SetWriter( iActiveWriter );
        }

    // Set settings to the handler
    if ( ret == KErrNone && iTraceCoreSettings != NULL )
     	{
     	aHandler.SetSettings( iTraceCoreSettings );
    	}
    OstTrace1( TRACE_FLOW, DTRACECORE_REGISTERHANDLER_EXIT, "< DTraceCore::RegisterHandler. Ret:%d", ret );

    return ret;
    }


/**
 * Unregisters a handler
 *
 * @param aHandler The handler
 */
void DTraceCore::UnregisterHandler( DTraceCoreHandler& aHandler )
    {
    // Find thiss handler from the list of handlers
    for ( TInt i = 0; i < iHandlers.Count(); i++ )
        {
        if ( iHandlers[ i ] == &aHandler )
            {
            OstTrace1( TRACE_NORMAL, DTRACECORE_UNREGISTERHANDLER_HANDLER_REMOVED, "DTraceCore::UnregisterHandler - Handler removed 0x%x", ( TUint )&aHandler );
            iHandlers.Remove( i );
            i = iHandlers.Count();
            }
        }
    }

/**
 * Registers the settings
 *
 * @param aSettings The settings saver
 * @return KErrNone if registration successful
 */
TInt DTraceCore::RegisterSettings( DTraceCoreSettings& aSettings )
    {
    iTraceCoreSettings = &aSettings;
    for ( TInt i = 0; i < iHandlers.Count(); i++ )
        {
        iHandlers[ i ]->SetSettings( iTraceCoreSettings );
        }
    return KErrNone;
    }


/**
 * Unregisters the settings
 *
 * @param aSettings The settings
 */
void DTraceCore::UnregisterSettings( DTraceCoreSettings& aSettings )
    {
    if ( &aSettings == iTraceCoreSettings )
        {
        iTraceCoreSettings = NULL;

        // Remove settings from all the handlers
        for ( TInt i = 0; i < iHandlers.Count(); i++ )
            {
            iHandlers[ i ]->SetSettings( NULL );
            }
        }
    }


/**
 * Registers a writer
 *
 * @param aWriter The writer
 */
TInt DTraceCore::RegisterWriter( DTraceCoreWriter& aWriter )
    {
    OstTrace1( TRACE_FLOW, DTRACECORE_REGISTERWRITER_ENTRY, "> DTraceCore::RegisterWriter 0x%x", ( TUint )&( aWriter ) );

    TInt ret = iWriters.Append( &aWriter );
    if ( ret == KErrNone )
        {
        // First writer that registers is selected as the active one
        if ( iActiveWriter == NULL )
            {
            iActiveWriter = &aWriter;
            SetWriterToHandlers();
            }
        }

    OstTrace1( TRACE_FLOW, DTRACECORE_REGISTERWRITER_EXIT, "< DTraceCore::RegisterWriter %d", ret );
    return ret;
    }


/**
 * Unregisters a writer
 *
 * @param aWriter The writer
 */
void DTraceCore::UnregisterWriter( DTraceCoreWriter& aWriter )
    {
    for ( TInt i = 0; i < iWriters.Count(); i++ )
        {
        if ( iWriters[ i ] == &aWriter )
            {
            OstTraceExt2( TRACE_NORMAL, DTRACECORE_UNREGISTERHANDLER_WRITER_UNREGISTERED, "DTraceCore::UnregisterWriter - Writer unregistered WriterType:%d Addr:0x%x", ( TInt )aWriter.GetWriterType(), ( TUint )&aWriter );
            iWriters.Remove( i );
            i = iWriters.Count();
            // If the active writer was removed, another writer needs to be activated
            if ( &aWriter == iActiveWriter )
                {
                SwitchToFirstWriter();
                }
            }
        }
    }


/**
 * Starts to use the first writer from the writers list
 */
void DTraceCore::SwitchToFirstWriter()
    {
    if ( iWriters.Count() > 0 )
        {
        iActiveWriter = iWriters[ 0 ];
        }
    else
        {
        iActiveWriter = NULL;
        }
    SetWriterToHandlers();
    }


/**
 * Starts to use a writer of the specific type. Does nothing if the writer is already active
 */
EXPORT_C TInt DTraceCore::SwitchToWriter( TWriterType aWriterType )
    {
    OstTrace1( TRACE_FLOW, DTRACECORE_SWITCHTOWRITER_ENTRY,"> DTraceCore::SwitchToWriter WriterType:%d", ( TUint )( aWriterType ) );
    TInt retval( KErrNone );

    // Remove writer
    if ( aWriterType == EWriterTypeNone )
        {
        if ( iActiveWriter != NULL )
            {
            iActiveWriter = NULL;
            SetWriterToHandlers();
            }
        }
    else
        {
        if ( iActiveWriter == NULL || iActiveWriter->GetWriterType() != aWriterType )
            {
            retval = KErrNotSupported;

            // Find the correct writer from the list
            for ( int i = 0; i < iWriters.Count() && retval != KErrNone; i++ )
                {
                DTraceCoreWriter* writer = iWriters[ i ];

                // revert this
                TWriterType wt = writer->GetWriterType();

                if ( wt == aWriterType )
                    {
                    iActiveWriter = writer;
                    retval = KErrNone;
                    }
                }

            // Set writer to all handlers
            if ( retval == KErrNone )
                {
                SetWriterToHandlers();
                }
            }
        }
    OstTrace1( TRACE_FLOW, DTRACECORE_SWITCHTOWRITER_EXIT, "< DTraceCore::SwitchToWriter %d", retval );
    return retval;
    }

/**
 * Sets the active writer to all registered handlers
 */
void DTraceCore::SetWriterToHandlers()
    {
    if ( iActiveWriter != NULL )
        {
        OstTraceExt2( TRACE_NORMAL, DTRACECORE_SETWRITERTOHANDLERS_WRITER_SELECTED, "DTraceCore::SetWriterToHandlers - Writer selected 0x%x WriterType:%d", (TUint) iActiveWriter, (TInt) iActiveWriter->GetWriterType() );
        }

    // PrepareSetWriter method is called with interrupts enabled
    // SetWriter is called with interrupts disabled to prevent tracing
    // while switching the writer
    for ( TInt i = 0; i < iHandlers.Count(); i++ )
        {
        iHandlers[ i ]->PrepareSetWriter( iActiveWriter );
        }
    //TODO: this is not SMP safe but it's rare case - update it later
    TInt interrupts = NKern::DisableAllInterrupts();
    for ( TInt j = 0; j < iHandlers.Count(); j++ )
        {
        iHandlers[ j ]->SetWriter( iActiveWriter );
        }
    NKern::RestoreInterrupts( interrupts );
    }

/**
 * Get current writer type
 *
 * @return Current writer type
 */
EXPORT_C TInt DTraceCore::GetCurrentWriterType()
    {
    TInt ret(0);
    ret = iActiveWriter->GetWriterType();
    return ret;
    }

/*
 * Destroy the static instance of tracecore
 */
EXPORT_C void DTraceCore::DestroyTraceCore()
    {
    TC_TRACE( ETraceLevelNormal, Kern::Printf( "StopTraceCore started" ) );
    delete DTraceCore::GetInstance();
    DTraceCore::iInstance = NULL;
    TC_TRACE( ETraceLevelNormal, Kern::Printf( "StopTraceCore Finished" ) );
    }


/**
 * Sets the "Trace Dropped" flag
 *
 * @param aTraceDropped ETrue if Trace Dropped, EFalse Otherwise
 */
EXPORT_C void DTraceCore::SetPreviousTraceDropped(TBool aTraceDropped)
    {
    iPreviousTraceDropped = aTraceDropped;
    }

/**
 * returns the state of the  "Trace Dropped" flag
 *
 * @param none
 */
EXPORT_C TBool DTraceCore::PreviousTraceDropped() const
    {
    return iPreviousTraceDropped;
    }


EXPORT_C DTraceCore* DTraceCore::CreateInstance()
    {
    if(iInstance == NULL)
      {
        DTraceCore* instance = new DTraceCore();
        if ( instance != NULL )
            {
            iInstance = instance;
            TInt ret = iInstance->Init();
            if(ret != KErrNone)
                {
                delete iInstance;
                iInstance = NULL;
                }
            }
        }

    return iInstance;
    }


EXPORT_C DTraceCoreWriter* DTraceCore::GetActiveWriter()
    {
    return iActiveWriter;
    }

/**
 * Reactivate all currently activated traces
 *
 * @param none
 * @return KErrNone if refresh successful
 */
TInt DTraceCore::RefreshActivations()
    {
    TInt ret = KErrNone;
    // Go through all activation interfaces from the list
    TInt activationsCount = iActivations.Count();
    for ( int i = 0; i < activationsCount; i++ )
        {
        TInt err = iActivations[ i ]->RefreshActivations();
        if (err != KErrNone)
            {
            ret = err;
            }
        }
    return ret;
    }
