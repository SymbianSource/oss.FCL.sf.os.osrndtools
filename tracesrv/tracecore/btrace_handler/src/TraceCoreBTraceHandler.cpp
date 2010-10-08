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
// Trace Core
//

#include "TraceCoreBTraceHandler.h"
#include "TraceCoreWriter.h"
#include "TraceCoreDebug.h"
#include "TraceCoreConstants.h"
#include "BTraceOstCategoryHandler.h"
#include "BTraceKernelCategoryHandler.h"
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "TraceCoreBTraceHandlerTraces.h"
#endif


/**
 * Static instance is needed when calling traces from handler function
 */
DTraceCoreBTraceHandler* DTraceCoreBTraceHandler::iInstance = NULL;


/**
 * Constructor
 */
DTraceCoreBTraceHandler::DTraceCoreBTraceHandler()
: iCategoryHandlers( NULL )
, iOstHandler( NULL )
, iKernelHandler( NULL )
	{
	}


/**
 * Destructor
 */
DTraceCoreBTraceHandler::~DTraceCoreBTraceHandler()
	{
    // Handlers are deleted first -> They call UnregisterCategoryHandler
	delete iOstHandler;
	delete iKernelHandler;
    delete[] iCategoryHandlers;
    DTraceCoreBTraceHandler::iInstance = NULL;
	}
	

/**
 * Initializes BTrace handler
 */
TInt DTraceCoreBTraceHandler::Init()
	{
    iCategoryHandlers = new DBTraceCategoryHandler*[ KBTraceCategoryCount ];
    TInt ret;
    if ( iCategoryHandlers != NULL )
        {
        memset( iCategoryHandlers, 0, sizeof ( DBTraceCategoryHandler* ) * KBTraceCategoryCount );
        // Registers this handler to TraceCore
        ret = Register();
        if ( ret == KErrNone )
            {
            // Registers the callback function to BTrace
            BTrace::SetHandler( BTraceHandlerFunc );
            DTraceCoreBTraceHandler::iInstance = this;
            }
        }
    else
        {
        ret = KErrNoMemory;
        }

    // Autogen, OST and Symbian kernel category handlers are integrated to TraceCore
    if ( ret == KErrNone )
        {

        ret = StartOstHandler();

        if ( ret == KErrNone )
            {
            ret = StartKernelHandler();
            }
        }
    
    TC_TRACE( ETraceLevelFlow, Kern::Printf( "<DTraceCoreBTraceHandler::Init - return %d", ret ) );
    return ret;
    }


    
/**
 * Starts the OST category handler
 */
TInt DTraceCoreBTraceHandler::StartOstHandler()
    {
    TInt ret = KErrNoMemory;
    iOstHandler = new DBTraceOstCategoryHandler();
    if ( iOstHandler != NULL )
        {
        // Init calls RegisterCategoryHandler
        ret = iOstHandler->Init();
        }
    TC_TRACE( ETraceLevelError,Kern::Printf( "DTraceCoreBTraceHandler::StartOstHandler - %d", ret ) );
    return ret;
    }


/**
 * Starts the kernel category handler
 */
TInt DTraceCoreBTraceHandler::StartKernelHandler()
    {
    TInt ret = KErrGeneral;
    if ( iOstHandler != NULL )
        {
        iKernelHandler = new DBTraceKernelCategoryHandler();
        if ( iKernelHandler != NULL )
            {
            // Init calls RegisterCategoryHandler
            ret = iKernelHandler->Init();
            }
        // Memory allocation failed
        else
            {
            ret = KErrNoMemory;
            }
        }
    // noelse

    TC_TRACE( ETraceLevelError, Kern::Printf("DTraceCoreBTraceHandler::StartKernelHandler - %d", ret ) );
    return ret;
    }


/**
 * Called before SetWriter with interrupts enabled
 * 
 * @param aWriter Pointer to writer
 */       
void DTraceCoreBTraceHandler::PrepareSetWriter( DTraceCoreWriter* aWriter )
    {
    if ( iCategoryHandlers != NULL )
        {
        DTraceCoreHandler::PrepareSetWriter( aWriter );
        // Delegates the writer to category handlers
        DBTraceCategoryHandler* previousHandler = NULL;
        for ( TInt i = 0; i < KBTraceCategoryCount; i++ )
            {
            DBTraceCategoryHandler* handler = iCategoryHandlers[ i ];
            if ( handler != NULL && handler != previousHandler )
                {
                handler->PrepareSetWriter( aWriter );
                previousHandler = handler;
                }
            }
        }
    }


/**
 * Sets the writer to be used for trace output
 * 
 * @param aWriter Pointer to writer
 */       
void DTraceCoreBTraceHandler::SetWriter( DTraceCoreWriter* aWriter )
    {
    OstTrace1( TRACE_FLOW, DTRACECOREBTRACEHANDLER_SETWRITER_ENTRY, "> DTraceCoreBTraceHandler::SetWriter 0x%x", ( TUint )&( aWriter ) );
    if (aWriter)
    	{
		OstTrace1( TRACE_INTERNALS, DTRACECOREBTRACEHANDLER_SETWRITER_WRITERTYPE, "Writer type: %d", aWriter->GetWriterType() );
		}
    
    if ( iCategoryHandlers != NULL )
        {
        DTraceCoreHandler::SetWriter( aWriter );
        // Delegates the writer to category handlers
        DBTraceCategoryHandler* previousHandler = NULL;
        for ( TInt i = 0; i < KBTraceCategoryCount; i++ )
            {
            DBTraceCategoryHandler* handler = iCategoryHandlers[ i ];
            if ( handler != NULL && handler != previousHandler )
                {
                handler->SetWriter( aWriter );
                previousHandler = handler;
                }
            }
        }
    }


/**
 * Sets settings
 * 
 * @param aSettings Pointer to settings
 */       
void DTraceCoreBTraceHandler::SetSettings( DTraceCoreSettings* aSettings )
    {
    OstTrace1( TRACE_FLOW, DTRACECOREBTRACEHANDLER_SETSETTINGS_ENTRY, "> DTraceCoreBTraceHandler::SetSettings 0x%x", ( TUint )&( aSettings ) );
    if ( iCategoryHandlers != NULL )
        {
        DTraceCoreHandler::SetSettings( aSettings );
        // Delegates the settings saver to category handlers
        DBTraceCategoryHandler* previousHandler = NULL;
        for ( TInt i = 0; i < KBTraceCategoryCount; i++ )
            {
            DBTraceCategoryHandler* handler = iCategoryHandlers[ i ];
            if ( handler != NULL && handler != previousHandler )
                {
                handler->SetSettings( aSettings );
                previousHandler = handler;
                }
            }
        }
    }



/**
 * Registers a category handler
 *
 * @param aCategory The category to be processed with the category handler
 * @param aHandler The handler which processes the category
 */
void DTraceCoreBTraceHandler::RegisterCategoryHandler( TUint8 aCategory, DBTraceCategoryHandler& aHandler )
    {
    OstTraceExt2( TRACE_FLOW, DTRACECOREBTRACEHANDLER_REGISTERCATEGORYHANDLER_ENTRY, "> DTraceCoreBTraceHandler::RegisterCategoryHandler. ID:0x%x Addr:0x%x", aCategory, ( TUint )&( aHandler ) );
    if ( iCategoryHandlers != NULL )
        {
        iCategoryHandlers[ aCategory ] = &aHandler;

        // BTrace kernel categories are not enabled by default
        // MF added - commented out this code

        if ( iWriter != NULL )
            {
            aHandler.SetWriter( iWriter );
            }
        if ( iSettings != NULL )
            {
            aHandler.SetSettings( iSettings );
            }
        OstTraceExt2( TRACE_NORMAL, DTRACECOREBTRACEHANDLER_REGISTERCATEGORYHANDLER_HANDLER_REGISTERED, "DTraceCoreBTraceHandler::RegisterCategoryHandler - Handler registered. ID:0x%x Addr:0x%x", aCategory, ( TUint )&( aHandler ) );
        }
    }


/**
 * Unregisters a category handler
 *
 * @param aCategory The category to be unregistered
 */
void DTraceCoreBTraceHandler::UnregisterCategoryHandler( TUint8 aCategory )
    {
    OstTrace1( TRACE_FLOW, DTRACECOREBTRACEHANDLER_UNREGISTERCATEGORYHANDLER_ENTRY, "> DTraceCoreBTraceHandler::UnregisterCategoryHandler. ID:0x%x", aCategory );
    
    // Unregister category handler
    if ( iCategoryHandlers != NULL )
        {
        iCategoryHandlers[ aCategory ] = NULL;
        BTrace::SetFilter( aCategory, 0 );
        OstTrace1( TRACE_NORMAL, DTRACECOREBTRACEHANDLER_UNREGISTERCATEGORYHANDLER_UNREGISTERED, "DTraceCoreBTraceHandler::UnregisterCategoryHandler - Handler unregistered. ID:0x%x", aCategory );
        }
    }


/**
 * Callback function that is registered to BTrace.
 * 
 * Tracing is not allowed from this method.
 *
 * @param aHeader BTrace header
 * @param aHeader2 Extra header data
 * @param aContext The thread context in which this function was called
 * @param a1 The first trace parameter
 * @param a2 The second trace parameter
 * @param a3 The third trace parameter
 * @param aExtra Extra trace data
 * @param aPc The program counter value
 * @return ETrue if trace was processed, EFalse if not
 */
TBool DTraceCoreBTraceHandler::BTraceHandlerFunc( TUint32 aHeader, TUint32 aHeader2, const TUint32 aContext, 
                                                  const TUint32 a1, const TUint32 a2, const TUint32 a3, 
                                                  const TUint32 aExtra, const TUint32 aPc )
    {
    TBool retval;
    DTraceCoreBTraceHandler* handler = DTraceCoreBTraceHandler::iInstance;
    if ( handler != NULL && handler->iWriter != NULL )
        {
        DBTraceCategoryHandler* categoryHandler = handler->GetCategoryHandler( aHeader );
        if ( categoryHandler != NULL )
            {
            retval = categoryHandler->HandleFrame( aHeader, aHeader2, aContext, a1, a2, a3, aExtra, aPc );
            }
        else
            {
            retval = EFalse;
            }
        }
    else
        {
        retval = EFalse;
        }
    return retval;
    }


/**
 * Gets the category handler for given BTrace header
 *
 * @param aHeader BTrace header
 */
inline DBTraceCategoryHandler* DTraceCoreBTraceHandler::GetCategoryHandler( TUint32 aHeader )
    {
    __ASSERT_DEBUG( iCategoryHandlers != NULL, Kern::Fault( "DTraceCoreBTraceHandler::GetCategoryHandler - NULL", KErrGeneral ) );
    return iCategoryHandlers[ ( aHeader >> ( BTrace::ECategoryIndex * KByteSize ) ) & KByteMask ];
    }

// End of File
