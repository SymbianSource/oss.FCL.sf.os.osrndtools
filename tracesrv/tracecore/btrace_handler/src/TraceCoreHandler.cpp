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

#include "TraceCoreHandler.h"
#include "TraceCore.h"
#include "TraceCoreDebug.h"
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "TraceCoreHandlerTraces.h"
#endif



/**
 * Constructor
 */
DTraceCoreHandler::DTraceCoreHandler()
: iWriter( NULL )
, iSettings( NULL )
    {
    }


/**
 * Destructor
 */
DTraceCoreHandler::~DTraceCoreHandler()
    {
    iWriter = NULL;
    Unregister();
    }
    

/**
 * Called before SetWriter with interrupts enabled
 * 
 * @param aWriter The new writer
 */       
void DTraceCoreHandler::PrepareSetWriter( DTraceCoreWriter* /*aWriter*/ )
    {
    }


/**
 * Sets active writer
 * 
 * @param aWriter The new writer
 */       
void DTraceCoreHandler::SetWriter( DTraceCoreWriter* aWriter )
    {
    iWriter = aWriter;
    }


/**
 * Sets active settings (saver)
 * 
 * @param aSettings The new settings
 */       
void DTraceCoreHandler::SetSettings( DTraceCoreSettings* aSettings )
    {
    iSettings = aSettings;
    }


/**
 * Registers this handler to TraceCore
 */
TInt DTraceCoreHandler::Register()
    {
    TInt ret = KErrGeneral;
    
    // Get TraceCore
    DTraceCore* traceCore = DTraceCore::GetInstance();
    if ( traceCore != NULL )
        {      
        // Register this as a handler
        ret = traceCore->RegisterHandler( *this );
        }
    OstTrace1( TRACE_IMPORTANT , DTRACECOREHANDLER_REGISTER_NO_TRACECORE, "DTraceCoreHandler::Register - %d", ret);
    return ret;
    }


/**
 * Unregisters this handler from trace core
 */
void DTraceCoreHandler::Unregister()
    {
    OstTrace0( TRACE_FLOW, DTRACECOREHANDLER_UNREGISTER_ENTRY, "> DTraceCoreHandler::Unregister");
    // Get TraceCore
    DTraceCore* traceCore = DTraceCore::GetInstance();
    if ( traceCore != NULL )
        {
        // Unregister this handler
        traceCore->UnregisterHandler( *this );
        }
    }

// End of File
