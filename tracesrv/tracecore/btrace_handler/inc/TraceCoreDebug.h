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
// TraceCore
//

#ifndef __TRACECOREDEBUG_H__
#define __TRACECOREDEBUG_H__

/**
 * Trace levels
 */
typedef enum
    {
    // Panics only
    ETraceLevelPanic,

    // Errors
    ETraceLevelError,

    // Warnings
    ETraceLevelWarning,

    // Normal traces
    ETraceLevelNormal,
    
    // Extra traces
    ETraceLevelExtra,
    
    // Flow traces
    ETraceLevelFlow,
    
    // Errors within trace functions
    ETraceLevelTraceError,
    
    // Flow within trace functions. Note that all BTrace calls throughout the system will
    // generate extra flow traces if this is set -> Slow...
    ETraceLevelTraceFlow,
    
    // All traces on
    ETraceLevelAll = KMaxTInt
    } TTraceLevel;
    
/**
 * Compile-time trace level for TraceCore debug purposes
 */
#define TC_TRACE_LEVEL ETraceLevelError

/**
 * Internal debug traces are on only in debug builds
 */
#ifdef _DEBUG
#ifdef TC_TRACE_LEVEL
#define TC_TRACE( level, trace ) /*lint -save -e506 -e522 -e774 -e834 -e960*/ { if ( level <= TC_TRACE_LEVEL ) { trace; } }	/*lint -restore*//* CodForChk_Dis_LengthyLine */
#else
#define TC_TRACE( level, trace ) {}
#endif
#else
#define TC_TRACE( level, trace ) {}
#endif // _DEBUG

// When defined, POST code is printed out
#define USE_POSTCODE

#ifdef USE_POSTCODE
#define POSTCODE( x ) {x}
#else
#define POSTCODE( x ) {}
#endif


#endif // __TRACECOREDEBUG_H__

// End of File
