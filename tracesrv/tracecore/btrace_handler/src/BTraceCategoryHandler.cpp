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

#include "BTraceCategoryHandler.h"
#include "TraceCore.h"
#include "TraceCoreBTraceHandler.h"
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "BTraceCategoryHandlerTraces.h"
#endif



/**
 * Constructor
 */	
EXPORT_C DBTraceCategoryHandler::DBTraceCategoryHandler()
: iWriter( NULL )
, iSettings( NULL )
    {
    }


/**
 * Destructor
 */	
EXPORT_C DBTraceCategoryHandler::~DBTraceCategoryHandler()
    {
    Unregister();
    iCategories.Close();
    }


/**
 * Called before SetWriter with interrupts enabled
 * 
 * @param aWriter The new writer
 */
EXPORT_C void DBTraceCategoryHandler::PrepareSetWriter( DTraceCoreWriter* /*aWriter*/ )
    {
    }


/**
 * Sets active writer
 * 
 * @param aWriter The new writer
 */
EXPORT_C void DBTraceCategoryHandler::SetWriter( DTraceCoreWriter* aWriter )
    {
    iWriter = aWriter;
    }


/**
 * Sets settings saver
 * 
 * @param aSettings Settings saver
 */    
EXPORT_C void DBTraceCategoryHandler::SetSettings( DTraceCoreSettings* aSettings )
    {
	iSettings = aSettings;
    }


/**
 * Adds a category to be registered
 *
 * @param aCategory The category
 */
EXPORT_C TInt DBTraceCategoryHandler::AddCategory( TUint8 aCategory )
    {
    OstTrace1( TRACE_BORDER, DBTRACECATEGORYHANDLER_ADDCATEGORY_ENTRY, "> DBTraceCategoryHandler::AddCategory 0x%x", aCategory );
    TInt ret = iCategories.Append( aCategory );
    OstTrace1( TRACE_BORDER, DBTRACECATEGORYHANDLER_ADDCATEGORY_EXIT, "< DBTraceCategoryHandler::AddCategory. Ret:%d", ret );
    return ret;
    }

    
/**
 * Registers this handler to BTrace
 */
EXPORT_C TInt DBTraceCategoryHandler::Register()
    {
    TInt ret = KErrNone;
    
    // Get BTrace handler
    DTraceCoreBTraceHandler* handler = DTraceCore::GetBTraceHandler();
    if ( handler != NULL )
        {       
        // Loop through all categories
        for ( TInt i = 0; i < iCategories.Count(); i++ )
            {
            handler->RegisterCategoryHandler( iCategories[ i ], *this );
            }
        ret = KErrNone;
        }
    else
        {
        ret = KErrGeneral;
        }
    OstTrace1( TRACE_BORDER, DBTRACECATEGORYHANDLER_REGISTER_EXIT, "< DBTraceCategoryHandler::Register. Ret:%d", ret );
    
    return ret;
    }
    

/**
 * Unregisters this handler from BTrace
 */    
void DBTraceCategoryHandler::Unregister()
    {
    OstTrace0( TRACE_FLOW, DBTRACECATEGORYHANDLER_UNREGISTER_ENTRY, "> DBTraceCategoryHandler::Unregister");
    // Get BTrace handler
    DTraceCoreBTraceHandler* handler = DTraceCore::GetBTraceHandler();
    if ( handler != NULL )
        {
        
        // Loop through all categories
        for ( TInt i = 0; i < iCategories.Count(); i++ )
            {
            handler->UnregisterCategoryHandler( iCategories[ i ] );
            }
        }
    }


// End of File
