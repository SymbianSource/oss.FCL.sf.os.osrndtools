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

#include "TraceCompilerTesterBasic.h"
#include "TraceCompilerTesterCommon1.h"
#include "TraceCompilerTesterCommon2.h"

#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "TraceCompilerTesterBasicTraces.h"
#include "TraceCompilerTesterCommon2Traces.h"
#endif

// Define group IDs. If TraceCompiler is missing, define these to get the code to compile.
// However, component still won't work properly.
#ifdef OST_TRACE_COMPILER_IN_USE
#define BASIC_OK_TEST TEST_BASIC_INT8 >> 16
#define ARRAY_OK_TEST TEST_ARRAY_INT8 >> 16
#define BASIC_TWO_SIMILAR_OK_TEST TEST_BASIC_SIMILAR2_INT8 >> 16
#define ARRAY_TWO_SIMILAR_OK_TEST TEST_ARRAY_SIMILAR2_INT8 >> 16
#define ENUM_OK_TEST TEST_ENUM_OK >> 16
#define TRACE_PERFORMANCE 0x4
#define TRACE_FLOW 0x7
#define INCLUDED_NON_SOURCE_FILE_TEST NON_SOURCE_FILE_TRACE1 >> 16
#define KEKKONEN TEXT1 >> 16
#else
#define BASIC_OK_TEST
#define ARRAY_OK_TEST
#define BASIC_TWO_SIMILAR_OK_TEST
#define ARRAY_TWO_SIMILAR_OK_TEST
#define ENUM_OK_TEST
#define EVENT_NO_VALUE_OK_TEST
#define EVENT_WITH_VALUE_OK_TEST
#define TRACE_PERFORMANCE
#define TRACE_FLOW
#define INCLUDED_NON_SOURCE_FILE_TEST
#define KEKKONEN
#endif

//- Local constants ---------------------------------------------------------

const TUint32 KComponentID = 0x20011112;

//- Macros ------------------------------------------------------------------


//- Member Functions --------------------------------------------------------

/**
 * Constructor
 */
DTraceCompilerTesterBasic::DTraceCompilerTesterBasic()
    {
    }

/**
 * EnergyMonitor Destructor
 */
DTraceCompilerTesterBasic::~DTraceCompilerTesterBasic()
    {

    }

/**
 * EnergyMonitor second-phase constructor
 *
 * @param
 * @return System-wide error code
 */
TInt DTraceCompilerTesterBasic::Construct()
    {

    TInt ret(KErrNone);
    
    Kern::Printf( "-------------- DTraceCompilerTesterBasic::Construct ----------------" );
    
    // Register notification receivers by using component ID and group IDs
    RegisterNotificationReceiver(KComponentID, BASIC_OK_TEST );
    RegisterNotificationReceiver(KComponentID, ARRAY_OK_TEST );
    RegisterNotificationReceiver(KComponentID, BASIC_TWO_SIMILAR_OK_TEST );
    RegisterNotificationReceiver(KComponentID, ARRAY_TWO_SIMILAR_OK_TEST );
    RegisterNotificationReceiver(KComponentID, ENUM_OK_TEST );
    RegisterNotificationReceiver(KComponentID, TRACE_PERFORMANCE );
    RegisterNotificationReceiver(KComponentID, TRACE_FLOW );
    RegisterNotificationReceiver(KComponentID, INCLUDED_NON_SOURCE_FILE_TEST );
    RegisterNotificationReceiver(KComponentID, KEKKONEN );

    return ret;
    }

/**
 * Callback function for Trace Activation
 * 
 * @param aComponentId
 * @param aGroupId         
 */
void DTraceCompilerTesterBasic::TraceActivated(TUint32 aComponentId,
        TUint16 aGroupId)
    {
    Kern::Printf(
            "DTraceCompilerTesterBasic::TraceActivated - ComponentId = 0x%x, GroupId = 0x%x",
            aComponentId, aGroupId);

    if (aGroupId == BASIC_OK_TEST)
        {
        Kern::Printf(
                "--------------TraceCompilerTesterBasic Array Basic START----------------");
        TUint8 unicodeStr[10];
        unicodeStr[0] = 'S';
        unicodeStr[1] = '\0';
        unicodeStr[2] = 'T';
        unicodeStr[3] = '\0';
        unicodeStr[4] = 'R';
        unicodeStr[5] = '\0';
        unicodeStr[6] = '1';
        unicodeStr[7] = '\0';
        unicodeStr[8] = '6';
        unicodeStr[9] = '\0';
        TPtrC8 unicodePtr(unicodeStr, 10);

        OstTraceExt1( BASIC_OK_TEST, TEST_BASIC_INT8, "int8: %hhd", ( TInt8 )1 );
        OstTraceExt1( BASIC_OK_TEST, TEST_BASIC_INT16, "int16: %hd", ( TInt16 )2 );
        OstTrace1( BASIC_OK_TEST, TEST_BASIC_INT32, "int32: %d", ( TInt )3 );
        OstTraceExt1( BASIC_OK_TEST, TEST_BASIC_INT64, "int64: %Ld", ( TInt64 )4 );
        OstTraceExt1( BASIC_OK_TEST, TEST_BASIC_UINT8, "uint8: %hhu", ( TUint16 )5 );
        OstTraceExt1( BASIC_OK_TEST, TEST_BASIC_UINT16, "uint16: %hu", ( TUint16 )6 );
        OstTrace1( BASIC_OK_TEST, TEST_BASIC_UINT32, "uint32: %u", ( TUint32 )7 );
        OstTraceExt1( BASIC_OK_TEST, TEST_BASIC_UINT64, "uint64: %Lu", ( TUint64 )8 );
        OstTraceExt1( BASIC_OK_TEST, TEST_BASIC_HEX8, "hex8: %hhx", ( TUint8 )9 );
        OstTraceExt1( BASIC_OK_TEST, TEST_BASIC_HEX16, "hex16: %hx", ( TUint16 )10 );
        OstTrace1( BASIC_OK_TEST, TEST_BASIC_HEX32, "hex32: %x", ( TUint32 )11 );
        OstTraceExt1( BASIC_OK_TEST, TEST_BASIC_HEX64, "hex64: %Lx", ( TUint64 )12 );
        OstTraceExt1( BASIC_OK_TEST, TEST_BASIC_STRING8, "ascii: %s", _L8( "STR8" ) );
        // In kernel unicode strings do not exist -> This uses const TDesC8&
        OstTraceExt1( BASIC_OK_TEST, TEST_BASIC_STRING16, "unicode: %S", unicodePtr );
        OstTraceExt1( BASIC_OK_TEST, TEST_BASIC_FFIX, "ffix: %f", 0.1 );
        OstTraceExt1( BASIC_OK_TEST, TEST_BASIC_FEXP, "fexp: %e", 0.2 );
        OstTraceExt1( BASIC_OK_TEST, TEST_BASIC_FOPT, "fopt: %g", 0.3 );
        Kern::Printf( "--------------TraceCompilerTesterBasic Array Basic END----------------" );
        }
    else if (aGroupId == ARRAY_OK_TEST)
        {
        
      
        Kern::Printf( "--------------TraceCompilerTesterBasic Array Test START----------------" );
        TInt buf8Len = 29;
        TInt8 buf8[ 29 ];
        TInt index = 0;
        TInt value = -17;
        while ( index < buf8Len )
            {
            buf8[ index++ ] = value;
            value++;
            }
        TInt buf16Len = 11;
        TInt16 buf16[ 11 ];
        index = 0;
        value = -3;
        while ( index < buf16Len )
            {
            buf16[ index++ ] = value;
            value += 7;
            }
        TInt buf32Len = 7;
        TInt buf32[ 7 ];
        index = 0;
        value = -73;
        while ( index < buf32Len )
            {
            buf32[ index++ ] = value;
            value += 150;
            }
        TInt buf64Len = 5;
        TInt64 buf64[ 5 ];
        index = 0;
        value = -100000000L;
        while ( index < buf64Len )
            {
            buf64[ index++ ] = value;
            value += 100000000L;
            }

#ifdef __WINS__
        TInt bufFloatLen = 7;
        TReal bufFloat[ 7 ];
        index = 0;
        TReal val = -0.5;
        while ( index < bufFloatLen )
            {
            bufFloat[ index++ ] = val;
            val += 1.5;
            }
#endif
        OstTraceExt1( ARRAY_OK_TEST, TEST_ARRAY_INT8, "int8 array: %{int8[]}",
                TOstArray< TInt8>( ( TInt8* )buf8, buf8Len ) );
        OstTraceExt1( ARRAY_OK_TEST, TEST_ARRAY_INT16, "int16 array: %{int16[]}",
                TOstArray< TInt16>( ( TInt16* )buf16, buf16Len ) );
        OstTraceExt1( ARRAY_OK_TEST, TEST_ARRAY_INT32, "int32 array: %{int32[]}",
                TOstArray< TInt>( ( TInt* )buf32, buf32Len ) );
        OstTraceExt1( ARRAY_OK_TEST, TEST_ARRAY_INT64, "int64 array: %{int64[]}",
                TOstArray< TInt64>( ( TInt64* )buf64, buf64Len ) );
        OstTraceExt1( ARRAY_OK_TEST, TEST_ARRAY_UINT8, "uint8 array: %{uint8[]}",
                TOstArray< TUint8>( ( TUint8* )buf8, buf8Len ) );
        OstTraceExt1( ARRAY_OK_TEST, TEST_ARRAY_UINT16, "uint16 array: %{uint16[]}",
                TOstArray< TUint16>( ( TUint16* )buf16, buf16Len ) );
        OstTraceExt1( ARRAY_OK_TEST, TEST_ARRAY_UINT32, "uint32 array: %{uint32[]}",
                TOstArray< TUint>( ( TUint* )buf32, buf32Len ) );
        OstTraceExt1( ARRAY_OK_TEST, TEST_ARRAY_UINT64, "uint64 array: %{uint64[]}",
                TOstArray< TUint64>( ( TUint64* )buf64, buf64Len ) );
        OstTraceExt1( ARRAY_OK_TEST, TEST_ARRAY_HEX8, "hex8 array: %{hex8[]}",
                TOstArray< TUint8>( ( TUint8* )buf8, buf8Len ) );
        OstTraceExt1( ARRAY_OK_TEST, TEST_ARRAY_HEX16, "hex16 array: %{hex16[]}",
                TOstArray< TUint16>( ( TUint16* )buf16, buf16Len ) );
        OstTraceExt1( ARRAY_OK_TEST, TEST_ARRAY_HEX32, "hex32 array: %{hex32[]}",
                TOstArray< TUint>( ( TUint* )buf32, buf32Len ) );
        OstTraceExt1( ARRAY_OK_TEST, TEST_ARRAY_HEX64, "hex64 array: %{hex64[]}",
                TOstArray< TUint64>( ( TUint64* )buf64, buf64Len ) );

#ifdef __WINS__
        OstTraceExt1( ARRAY_OK_TEST, TEST_ARRAY_FFIX, "ffix array: %{ffix[]}",
                TOstArray< TReal>( bufFloat, bufFloatLen ) );
        OstTraceExt1( ARRAY_OK_TEST, TEST_ARRAY_FEXP, "fexp array: %{fexp[]}",
                TOstArray< TReal>( bufFloat, bufFloatLen ) );
        OstTraceExt1( ARRAY_OK_TEST, TEST_ARRAY_FOPT, "fopt array: %{fopt[]}",
                TOstArray< TReal>( bufFloat, bufFloatLen ) );
#endif
        Kern::Printf( "--------------TraceCompilerTesterBasic Array Test END----------------" );
        }
    else if (aGroupId == BASIC_TWO_SIMILAR_OK_TEST)
        {
        Kern::Printf( "--------------TraceCompilerTesterBasic Basic Two Similar Test START----------------" );
        OstTraceExt2( BASIC_TWO_SIMILAR_OK_TEST, TEST_BASIC_SIMILAR2_INT8,
                "int8: %hhd %hhd", ( TInt8 )1, ( TInt8 )1 );
        OstTraceExt2( BASIC_TWO_SIMILAR_OK_TEST, TEST_BASIC_SIMILAR2_INT16,
                "int16: %hd, %hd", ( TInt16 )2, ( TInt16 )2 );
        OstTraceExt2( BASIC_TWO_SIMILAR_OK_TEST, TEST_BASIC_SIMILAR2_INT32,
                "int32: %d, %d", ( TInt )3, ( TInt )3 );
        OstTraceExt2( BASIC_TWO_SIMILAR_OK_TEST, TEST_BASIC_SIMILAR2_INT64,
                "int64: %Ld, %Ld", ( TInt64 )4, ( TInt64 )4 );
        OstTraceExt2( BASIC_TWO_SIMILAR_OK_TEST, TEST_BASIC_SIMILAR2_UINT8,
                "uint8: %hhu, %hhu", ( TUint8 )5, ( TUint8 )5 );
        OstTraceExt2( BASIC_TWO_SIMILAR_OK_TEST, TEST_BASIC_SIMILAR2_UINT16,
                "uint16: %hu, %hu", ( TUint16 )6, ( TUint16 )6 );
        OstTraceExt2( BASIC_TWO_SIMILAR_OK_TEST, TEST_BASIC_SIMILAR2_UINT32,
                "uint32: %u, %u", ( TUint )7, ( TUint )7 );
        OstTraceExt2( BASIC_TWO_SIMILAR_OK_TEST, TEST_BASIC_SIMILAR2_UINT64,
                "uint64: %Lu, %Lu", ( TUint64 )8, ( TUint64 )8 );
        OstTraceExt2( BASIC_TWO_SIMILAR_OK_TEST, TEST_BASIC_SIMILAR2_HEX8,
                "hex8: %hhx, %hhx", ( TUint8 )9, ( TUint8 )9 );
        OstTraceExt2( BASIC_TWO_SIMILAR_OK_TEST, TEST_BASIC_SIMILAR2_HEX16,
                "hex16: %hx, %hx", ( TUint16 )10, ( TUint16 )10 );
        OstTraceExt2( BASIC_TWO_SIMILAR_OK_TEST, TEST_BASIC_SIMILAR2_HEX32,
                "hex32: %x, %x", ( TUint )11, ( TUint )11 );
        OstTraceExt2( BASIC_TWO_SIMILAR_OK_TEST, TEST_BASIC_SIMILAR2_HEX64,
                "hex64: %Lx, %Lx", ( TUint64 )12, ( TUint64 )12 );
        OstTraceExt2( BASIC_TWO_SIMILAR_OK_TEST, TEST_BASIC_SIMILAR2_STRING8,
                "ascii: %s, %s", _L8( "STR8" ), _L8( "STR8" ) );
        OstTraceExt2( BASIC_TWO_SIMILAR_OK_TEST, TEST_BASIC_SIMILAR2_FFIX,
                "ffix: %f, %f", 0.1, 0.1 );
        OstTraceExt2( BASIC_TWO_SIMILAR_OK_TEST, TEST_BASIC_SIMILAR2_FEXP,
                "fexp: %e, %f", 0.2, 0.2 );
        OstTraceExt2( BASIC_TWO_SIMILAR_OK_TEST, TEST_BASIC_SIMILAR2_FOPT,
                "fopt: %g, %g", 0.3, 0.3 );
        Kern::Printf( "--------------TraceCompilerTesterBasic Basic Two Similar Test END----------------" );
        }
    else if (aGroupId == ARRAY_TWO_SIMILAR_OK_TEST)
        {
        Kern::Printf( "--------------TraceCompilerTesterBasic Array Two Similar Test START----------------" );
        TInt buf8Len = 29;
        TInt8 buf8[ 29 ];
        TInt index = 0;
        TInt value = -17;
        while ( index < buf8Len )
            {
            buf8[ index++ ] = value;
            value++;
            }
        TInt buf16Len = 11;
        TInt16 buf16[ 11 ];
        index = 0;
        value = -3;
        while ( index < buf16Len )
            {
            buf16[ index++ ] = value;
            value += 7;
            }
        TInt buf32Len = 7;
        TInt buf32[ 7 ];
        index = 0;
        value = -73;
        while ( index < buf32Len )
            {
            buf32[ index++ ] = value;
            value += 150;
            }
        TInt buf64Len = 5;
        TInt64 buf64[ 5 ];
        index = 0;
        value = -100000000L;
        while ( index < buf64Len )
            {
            buf64[ index++ ] = value;
            value += 100000000L;
            }

#ifdef __WINS__
        TInt bufFloatLen = 7;
        TReal bufFloat[ 7 ];
        index = 0;
        TReal val = -0.5;
        while ( index < bufFloatLen )
            {
            bufFloat[ index++ ] = val;
            val += 1.5;
            }
#endif

        OstTraceExt2( ARRAY_TWO_SIMILAR_OK_TEST, TEST_ARRAY_SIMILAR2_INT8, "int8: %{int8[]} %{int8[]}",
                TOstArray< TInt8>( ( TInt8* )buf8, buf8Len ), TOstArray< TInt8>( ( TInt8* )buf8, buf8Len ) );
        OstTraceExt2( ARRAY_TWO_SIMILAR_OK_TEST, TEST_ARRAY_SIMILAR2_INT16, "int16: %{int16[]} %{int16[]}",
                TOstArray< TInt16>( ( TInt16* )buf16, buf16Len ), TOstArray< TInt16>( ( TInt16* )buf16, buf16Len ) );
        OstTraceExt2( ARRAY_TWO_SIMILAR_OK_TEST, TEST_ARRAY_SIMILAR2_INT32, "int32: %{int32[]} %{int32[]}",
                TOstArray< TInt>( ( TInt* )buf32, buf32Len ), TOstArray< TInt>( ( TInt* )buf32, buf32Len ) );
        OstTraceExt2( ARRAY_TWO_SIMILAR_OK_TEST, TEST_ARRAY_SIMILAR2_INT64, "int64: %{int64[]} %{int64[]}",
                TOstArray< TInt64>( ( TInt64* )buf64, buf64Len ), TOstArray< TInt64>( ( TInt64* )buf64, buf64Len ) );
        OstTraceExt2( ARRAY_TWO_SIMILAR_OK_TEST, TEST_ARRAY_SIMILAR2_UINT8, "uint8: %{uint8[]} %{uint8[]}",
                TOstArray< TUint8>( ( TUint8* )buf8, buf8Len ), TOstArray< TUint8>( ( TUint8* )buf8, buf8Len ) );
        OstTraceExt2( ARRAY_TWO_SIMILAR_OK_TEST, TEST_ARRAY_SIMILAR2_UINT16, "uint16: %{uint16[]} %{uint16[]}",
                TOstArray< TUint16>( ( TUint16* )buf16, buf16Len ), TOstArray< TUint16>( ( TUint16* )buf16, buf16Len ) );
        OstTraceExt2( ARRAY_TWO_SIMILAR_OK_TEST, TEST_ARRAY_SIMILAR2_UINT32, "uint32: %{uint32[]} %{uint32[]}",
                TOstArray< TUint>( ( TUint* )buf32, buf32Len ), TOstArray< TUint>( ( TUint* )buf32, buf32Len ) );
        OstTraceExt2( ARRAY_TWO_SIMILAR_OK_TEST, TEST_ARRAY_SIMILAR2_UINT64, "uint64: %{uint64[]} %{uint64[]}",
                TOstArray< TUint64>( ( TUint64* )buf64, buf64Len ), TOstArray< TUint64>( ( TUint64* )buf64, buf64Len ) );
        OstTraceExt2( ARRAY_TWO_SIMILAR_OK_TEST, TEST_ARRAY_SIMILAR2_HEX8, "hex8: %{hex8[]} %{hex8[]}",
                TOstArray< TUint8>( ( TUint8* )buf8, buf8Len ), TOstArray< TUint8>( ( TUint8* )buf8, buf8Len ) );
        OstTraceExt2( ARRAY_TWO_SIMILAR_OK_TEST, TEST_ARRAY_SIMILAR2_HEX16, "hex16: %{hex16[]} %{hex16[]}",
                TOstArray< TUint16>( ( TUint16* )buf16, buf16Len ), TOstArray< TUint16>( ( TUint16* )buf16, buf16Len ) );
        OstTraceExt2( ARRAY_TWO_SIMILAR_OK_TEST, TEST_ARRAY_SIMILAR2_HEX32, "hex32: %{hex32[]} %{hex32[]}",
                TOstArray< TUint>( ( TUint* )buf32, buf32Len ), TOstArray< TUint>( ( TUint* )buf32, buf32Len ) );
        OstTraceExt2( ARRAY_TWO_SIMILAR_OK_TEST, TEST_ARRAY_SIMILAR2_HEX64, "hex64: %{hex64[]} %{hex64[]}",
                TOstArray< TUint64>( ( TUint64* )buf64, buf64Len ), TOstArray< TUint64>( ( TUint64* )buf64, buf64Len ) );

#ifdef __WINS__
        OstTraceExt2( ARRAY_TWO_SIMILAR_OK_TEST, TEST_ARRAY_SIMILAR2_FFIX, "ffix: %{ffix[]} %{ffix[]}",
                TOstArray< TReal>( ( TReal* )bufFloat, bufFloatLen ), TOstArray< TReal>( ( TReal* )bufFloat, bufFloatLen ) );
        OstTraceExt2( ARRAY_TWO_SIMILAR_OK_TEST, TEST_ARRAY_SIMILAR2_FEXP, "fexp: %{fexp[]} %{fexp[]}",
                TOstArray< TReal>( ( TReal* )bufFloat, bufFloatLen ), TOstArray< TReal>( ( TReal* )bufFloat, bufFloatLen ) );
        OstTraceExt2( ARRAY_TWO_SIMILAR_OK_TEST, TEST_ARRAY_SIMILAR2_FOPT, "fopt: %{fopt[]} %{fopt[]}",
                TOstArray< TReal>( ( TReal* )bufFloat, bufFloatLen ), TOstArray< TReal>( ( TReal* )bufFloat, bufFloatLen ) );

#endif         
        Kern::Printf( "--------------TraceCompilerTesterBasic Array Two Similar Test END----------------" );
        }
    else if (aGroupId == ENUM_OK_TEST)
        {
        Kern::Printf( "--------------TraceCompilerTesterBasic Enum Test START----------------" );
        TInt buf[3];
        buf[0] = KErrNone;
        buf[1] = KErrNotFound;
        buf[2] = KErrGeneral;
        TInt ret = KErrGeneral;
        OstTrace1( ENUM_OK_TEST, TEST_ENUM_OK, "Enum: %{TErrorCodes}", ret );
        OstTraceExt1( ENUM_OK_TEST, TEST_ENUM_ARRAY_OK, "Enum Array: %{TErrorCodes[]}",
                TOstArray< TInt>( ( TInt* )buf, 3 ) );
        Kern::Printf( "--------------TraceCompilerTesterBasic Enum Test END----------------" );
        }
    else if (aGroupId == TRACE_PERFORMANCE)
        {
        Kern::Printf( "--------------TraceCompilerTesterBasic Event Test START----------------" );
       
        OstTraceEventStart0( EVENT_MY_EVENT_TRACE1_START, "Event1" );
        OstTraceEventStop( EVENT_MY_EVENT_TRACE1_STOP, "Event1", EVENT_MY_EVENT_TRACE1_START );

        OstTraceEventStart1( EVENT_MY_EVENT_TRACE2_START, "Event2", 123 );
        OstTraceEventStop( EVENT_MY_EVENT_TRACE2_STOP, "Event2", EVENT_MY_EVENT_TRACE2_START );
        
        Kern::Printf( "--------------TraceCompilerTesterBasic Event Test END----------------" );
        }
    else if (aGroupId == TRACE_FLOW)
        {
        // This test can also use to test function parameters
        Kern::Printf( "--------------TraceCompilerTesterBasic Common Source File Test START----------------");
        DTraceCompilerTesterCommon1* common1 = new DTraceCompilerTesterCommon1();
        TInt32 sum = common1->SumValues(1, 2, 3, 4);
        delete common1;
        Kern::Printf( "--------------TraceCompilerTesterBasic Common Source File Test END----------------");
        }
    else if (aGroupId == INCLUDED_NON_SOURCE_FILE_TEST)
        {
        // Print out traces from included non-source file that is defined in trace.properties file
        Kern::Printf( "--------------TraceCompilerTesterBasic Included Non-Source File Test START----------------" );

        TUint32 value1 = GetBigValue1000();
        TUint32 value2 = GetBigValue2000();

        Kern::Printf( "--------------TraceCompilerTesterBasic Included Non-Source File Test END----------------" );
        }
    else if (aGroupId == KEKKONEN)
        {
        Kern::Printf( "--------------TraceCompilerTesterBasic Kekkonen START----------------" );
        DTraceCompilerTesterCommon2* common2 = new DTraceCompilerTesterCommon2();
        common2->Kekkonen();
        delete common2;
        Kern::Printf( "--------------TraceCompilerTesterBasic Kekkonen END----------------" );
        }
    }

/**
 * Callback function for Trace Deactivation
 * 
 * @param aComponentId
 * @param aGroupId     
 */
void DTraceCompilerTesterBasic::TraceDeactivated(TUint32 aComponentId,
        TUint16 aGroupId)
    {
    Kern::Printf(
            "DTraceCompilerTesterBasic::TraceDeactivated - ComponentId = 0x%x, GroupId = 0x%x",
            aComponentId, aGroupId);
    }
/**
 * The entry point for a standard extension. Creates Long Trace Tester extension.
 *
 *  @return KErrNone, if successful
 */
DECLARE_STANDARD_EXTENSION()
    {

    TInt ret(KErrNone);

    DTraceCompilerTesterBasic* longTraceTester =
            new DTraceCompilerTesterBasic();

    if (longTraceTester == NULL)
        {
        ret = KErrNoMemory;
        }

    if (ret == KErrNone)
        {
        // construct instance
        ret = longTraceTester->Construct();
        }

    return ret;
    }
// End of File
