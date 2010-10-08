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

#include "TraceCore.h"
#include "TraceCoreSettings.h"
#include "TraceCoreDebug.h"
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "TraceCoreSettingsTraces.h"
#endif



/**
 * Constructor
 */
EXPORT_C DTraceCoreSettings::DTraceCoreSettings()
    {
    }


/**
 * Destructor
 */
EXPORT_C DTraceCoreSettings::~DTraceCoreSettings()
    {
    Unregister();
    }


/**
 * Registers this settings saver to TraceCore
 */
EXPORT_C TInt DTraceCoreSettings::Register()
    {
    TInt ret = KErrGeneral;
    
    // Get TraceCore
    DTraceCore* traceCore = DTraceCore::GetInstance();
    if ( traceCore != NULL )
        {        
        // Register to settings
        ret = traceCore->RegisterSettings( *this );
        }
    OstTrace1( TRACE_BORDER, DTRACECORESETTINGS_REGISTER_EXIT, "< DTraceCoreSettings::Register. Ret:%d", ret );
    return ret;
    }


/**
 * Unregisters this settings saver from TraceCore
 */
TInt DTraceCoreSettings::Unregister()
    {
    // Get TraceCore
    DTraceCore* traceCore = DTraceCore::GetInstance();
    TInt ret = KErrGeneral;
    if ( traceCore != NULL )
        {       
        // Unregister from settings
        traceCore->UnregisterSettings( *this );
        }
    OstTrace1( TRACE_FLOW, DTRACECORESETTINGS_UNREGISTER_EXIT, "< DTraceCoreSettings::Unregister. Ret:%d", ret );
    return ret;
    }



// End of File
