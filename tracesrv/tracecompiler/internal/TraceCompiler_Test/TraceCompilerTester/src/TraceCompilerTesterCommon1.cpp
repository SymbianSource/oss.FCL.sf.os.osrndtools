/*
* Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies).
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
#include <nkern.h>

#include "TraceCompilerTesterCommon1.h"

#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "TraceCompilerTesterCommon1Traces.h"
#endif


/**
 * Constructor
 */
DTraceCompilerTesterCommon1::DTraceCompilerTesterCommon1()
	{
    }
	
/**
 * Destructor
 */
DTraceCompilerTesterCommon1::~DTraceCompilerTesterCommon1()
    {

    }


/**
 * Sum values
 * 
 * @param aValue1
 * @param aValue2
 * @param aValue3
 * @param aValue4
 * @return sum of the values          
 */    
TInt32 DTraceCompilerTesterCommon1::SumValues( TInt aValue1, TInt aValue2, TUint8 aValue3, TUint8 aValue4 )
    {
    OstTraceFunctionEntryExt( DTRACECOMPILERTESTERCOMMON1_SUMVALUES_ENTRY, this );
    TInt32 retVal = aValue1 + aValue2 +aValue3 + aValue4;
    OstTraceFunctionExitExt( DTRACECOMPILERTESTERCOMMON1_SUMVALUES_EXIT, this, ( TInt )( retVal ) );
    return retVal;				
    }

// End of File
