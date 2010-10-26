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

#ifndef __TRACECOMPILERADVANCED_INL__
#define __TRACECOMPILERADVANCED_INL__

#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "TraceCompilerTesterAdvancedTraces.h"
#endif

inline void DTraceCompilerTesterAdvanced::HelloWorld()
    {
    OstTrace0( INCLUDED_NON_SOURCE_FILE_TEST, HELLO_WORLD, "Hello world!" );
    }
    
#endif
