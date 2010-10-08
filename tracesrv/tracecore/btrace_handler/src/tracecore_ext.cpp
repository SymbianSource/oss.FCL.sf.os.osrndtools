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

#include <e32def.h>
#include <e32btrace.h>

#include "TraceCore.h"
#include "TraceCoreDebug.h"

DECLARE_STANDARD_EXTENSION()
    {    
    TC_TRACE( ETraceLevelNormal, Kern::Printf( "DECLARE_STANDARD_EXTENSION Start Trace Core" ) );
    TInt ret;
    DTraceCore* tracecore = DTraceCore::CreateInstance();
    if ( tracecore != NULL )
        {
        ret = KErrNone;
        POSTCODE( Kern::Printf("[POST][TraceCore][Start][OK]"); )
        }
    else
        {
        //TODO: don't fail phone's startup sequence when DTraceCore obj is not created
        //TODO: fail only in debug mode
        ret = KErrNoMemory;
        POSTCODE( Kern::Printf("[POST][TraceCore][Start][FAIL][%d]", ret ); )
        }
    
    return ret;
    }
