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
// TraceCore constants
//
// @internalTechnology
// Please do not use this header file as its content may chnage in future.
//

#ifndef __TRACECORECONSTANTS_H__
#define __TRACECORECONSTANTS_H__

#include <opensystemtrace_types.h>


/**
 * Enables statistics of media writer
 */
#define MEDIA_WRITER_STATISTICS

/**
 * Disables printf traces from media writer
 */
// #define MEDIA_WRITER_TRACE_DEBUG

/**
 * Trace bitmap default value
 * 0xff -> All traces enabled at boot
 * 0x00 -> All traces disabled at boot
 */
const TUint8 KDefaultTraceBitmapByte = 0x00;

/**
 * Byte size
 */
const TInt KByteSize = 8;

/**
 * Byte mask
 */
const TUint KByteMask = 0xFF;

/** 
 * Default DFC queue priority
 */
const TUint8 KDefaultDfcPriority = 3; 

/**
 * BTrace category ID for Autogen traces
 */
const TUint KCategoryNokiaAutogen = 0xA0;

/**
 * BTrace category ID for branch coverage traces
 */
const TUint KCategoryNokiaBranchCoverage = 0xA1;

/**
 * OST component UID for Symbian kernel hooks
 */
const TUint32 KKernelHooksOSTComponentUID = 0x2001022D;

/**
 * OST component UID for Autogen traces
 */
const TUint32 KOldNokiaAutogenOSTComponentUID = 0;

/**
 * Constants for BTrace header
 */
const TUint KHeaderSize = 4;
const TUint KHeader2Size = 4;
const TUint KTimestampSize = 4;
const TUint KContextIdSize = 4;
const TUint KExtraSize = 4;
const TUint KPcSize = 4;
const TUint KA1Size = 4;
const TUint KA2Size = 4;
const TUint KA3Size = 4;
const TUint KComponentIDSize = 4;

/**
 * Number of categories in BTrace
 */
const TInt KBTraceCategoryCount = 256;

/**
 * Represents all groups of a component when registering with notifier
 */
const TUint KAllGroups = 0;


#ifdef __WINS__
/**
 * Endian swapper for trace ID's
 */
#define SWAP_DATA( num ) ( num )

/**
 * Endian swapper for trace data
 */
#define SWAP_ID( num ) ( \
    ( ( ( num ) & 0xff000000 ) >> 24 ) | ( ( ( num ) & 0x00ff0000 ) >> 8  ) | \
    ( ( ( num ) & 0x0000ff00 ) <<  8 ) | ( ( ( num ) & 0x000000ff ) << 24 ) )

#else


/**
 * Endian swapper for trace ID's
 */
#define SWAP_ID( num ) ( num )

/**
 * Endian swapper for trace data
 */
#define SWAP_DATA( num ) ( \
    ( ( ( num ) & 0xff000000 ) >> 24 ) | ( ( ( num ) & 0x00ff0000 ) >> 8  ) | \
    ( ( ( num ) & 0x0000ff00 ) <<  8 ) | ( ( ( num ) & 0x000000ff ) << 24 ) )
#endif

const TUint8 KMinKernelCategory = BTrace::EThreadIdentification;

const TUint8 KMaxKernelCategory = BTrace::EPlatformSpecificFirst - 1;

const TUint8 KMaxCategory = KMaxGroupId;


#endif

// End of File
