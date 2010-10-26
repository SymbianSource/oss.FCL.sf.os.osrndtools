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

#include "TraceCompilerTesterAdvanced.h"
#include "TraceCompilerTesterCommon1.h"
#include "TraceCompilerTesterCommon2.h"

#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "TraceCompilerTesterAdvancedTraces.h"
#include "TraceCompilerTesterCommon2Traces.h"
#endif

// Define group IDs. If TraceCompiler is missing, define these to get the code to compile.
// However, component still won't work properly.
#ifdef OST_TRACE_COMPILER_IN_USE
#define SHORT_TRACE_TEST ASCII1 >> 16
#define LONG_TRACE_TEST ASCII300 >> 16
#define POINTER_PARAMETER_TEST POINTER_PARAMETER_TRACE1 >> 16
#define INCLUDED_NON_SOURCE_FILE_TEST HELLO_WORLD >> 16
#define TRACE_FLOW 0x7
#define KEKKONEN TEXT1 >> 16
#define TRACE_STATE 0x5
#define TRACE_DATA TRACE_DATA_TRACE >> 16
#define POST_CODE_TEST POST1 >> 16
#else
#define SHORT_TRACE_TEST
#define LONG_TRACE_TEST
#define POINTER_PARAMETER_TEST
#define INCLUDED_NON_SOURCE_FILE_TEST 
#define TRACE_FLOW
#define KEKKONEN
#define TRACE_STATE
#define TRACE_DATA
#define POST_CODE_TEST
#endif


//- Local constants ---------------------------------------------------------

const TUint32 KComponentID = 0x20011111;

_LIT8(buf1, "1");
_LIT8(buf2, "12");
_LIT8(buf3, "123");
_LIT8(buf4, "1234");
_LIT8(buf60, "123456789_123456789_123456789_123456789_123456789_123456789X");
_LIT8(buf300, "123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789X");
_LIT8(buf333, "123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_12X");
_LIT8(buf512, "123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789Y1X");
_LIT8(buf600, "123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789X");
 
//- Macros ------------------------------------------------------------------


//- Member Functions --------------------------------------------------------

/**
 * Constructor
 */
DTraceCompilerTesterAdvanced::DTraceCompilerTesterAdvanced()
	{
    }
	
/**
 * EnergyMonitor Destructor
 */
DTraceCompilerTesterAdvanced::~DTraceCompilerTesterAdvanced()
    {
    }

/**
 * EnergyMonitor second-phase constructor
 *
 * @param
 * @return System-wide error code
 */
TInt DTraceCompilerTesterAdvanced::Construct()
    {
    Kern::Printf( "-------------- DTraceCompilerTesterAdvanced::Construct ----------------" );    
    TInt ret( KErrNone );
    
    // Register notification receiver by using just component ID
    RegisterNotificationReceiver( KComponentID);
    
    return ret;
    }

/**
 * Callback function for Trace Activation
 * 
 * @param aComponentId
 * @param aGroupId         
 */   
void DTraceCompilerTesterAdvanced::TraceActivated( TUint32 aComponentId, TUint16 aGroupId  )
    {
    
    Kern::Printf( "DTraceCompilerTesterAdvanced::TraceActivated - ComponentId = 0x%x, GroupId = 0x%x", aComponentId, aGroupId ); 
    
    if (aGroupId == POINTER_PARAMETER_TEST)
        {
        // Print out pointer values of two NTimers by using %p tag 
        Kern::Printf( "--------------TraceCompilerTesterAdvanced Pointer Parameter Test START----------------" ); 
        NTimer* myNTimer1 = new NTimer();
        NTimer* myNTimer2 = new NTimer();
        const NTimer* myNTimer3 = new NTimer();
        OstTraceExt2( POINTER_PARAMETER_TEST, POINTER_PARAMETER_TRACE1, "Print out pointer values of two NTimers;myNTimer1=%p;myNTimer2=%p", myNTimer1, myNTimer2 );
        OstTraceExt1( POINTER_PARAMETER_TEST, POINTER_PARAMETER_TRACE2, "Print out pointer values of one const NTimers;myNTimer3=%p", myNTimer3 );
        delete myNTimer1;
        delete myNTimer2;
        delete myNTimer3;
        Kern::Printf( "--------------TraceCompilerTesterAdvanced Pointer Parameter Test END----------------" ); 
        }
    else if (aGroupId == LONG_TRACE_TEST)
        {
        // Print out n times traces those lengths are 60, 300, 512 and 600 characters.
		// Print out also some traces those parameter total length is more than 512
        Kern::Printf( "--------------TraceCompilerTesterAdvanced Long Trace Test START----------------" ); 
        
        TInt i=0;
	    TUint numberOfTraces = 1;
	    TInt32 freq = NKern::FastCounterFrequency();
        TUint32 startTime;
	    TUint32 endTime;
	    TUint32 time;
	    
	    TUint timeTestNumber1 = 0;
        TUint timeTestNumber2 = 0;
        TUint timeTestNumber3 = 0;
        TUint timeTestNumber4 = 0;
        TUint timeTestNumber5 = 0;
        TUint timeTestNumber6 = 0;
        TUint timeTestNumber7 = 0;
        // ---------------------------- Ascii60 ----------------------------	    
	    startTime = NKern::FastCounter();
	    Kern::Printf( "--------------TraceCompilerTesterAdvanced Long Trace Test : Ascii60----------------" ); 	    
	    for (i=0; i < numberOfTraces; i++)
	        {
            OstTraceExt1( LONG_TRACE_TEST, ASCII60, "Ascii60: %s", buf60);
            }
        endTime = NKern::FastCounter();
        time = (endTime - startTime) * 1000 / freq;

        timeTestNumber1 = time;
        // ---------------------------- Ascii60 ----------------------------
        
	    // ---------------------------- Ascii300 ----------------------------	    
	    startTime = NKern::FastCounter();
	    Kern::Printf( "--------------TraceCompilerTesterAdvanced Long Trace Test : Ascii300----------------" ); 
	    for (i=0; i < numberOfTraces; i++)
	        {
            OstTraceExt1( LONG_TRACE_TEST, ASCII300, "Ascii300: %s", buf300);
            }
        endTime = NKern::FastCounter();
        time = (endTime - startTime) * 1000 / freq;

        timeTestNumber2 = time;
        // ---------------------------- Ascii300 ----------------------------
        
        // ---------------------------- Ascii512 ----------------------------	    
	    startTime = NKern::FastCounter();
	    Kern::Printf( "--------------TraceCompilerTesterAdvanced Long Trace Test : Ascii512----------------" ); 
	    for (i=0; i < numberOfTraces; i++)
	        {
            OstTraceExt1( LONG_TRACE_TEST, ASCII512, "Ascii512: %s", buf512);
            }
        endTime = NKern::FastCounter();
        time = (endTime - startTime) * 1000 / freq;

        timeTestNumber3 = time;
        // ---------------------------- Ascii512 ----------------------------            

	    // ---------------------------- Ascii600 ----------------------------	    
	    startTime = NKern::FastCounter();
	    Kern::Printf( "--------------TraceCompilerTesterAdvanced Long Trace Test : Ascii600----------------" ); 
	    for (i=0; i < numberOfTraces; i++)
	        {
            OstTraceExt1( LONG_TRACE_TEST, ASCII600, "Ascii600: %s", buf600);
            }
        endTime = NKern::FastCounter();
        time = (endTime - startTime) * 1000 / freq;

        timeTestNumber4 = time;
        // ---------------------------- Ascii600 ----------------------------    

        // ---------------------------- Ascii300 + Ascii512 ----------------------------       
        startTime = NKern::FastCounter();
        Kern::Printf( "--------------TraceCompilerTesterAdvanced Long Trace Test : Ascii300 + Ascii512----------------" ); 
        for (i=0; i < numberOfTraces; i++)
            {
            OstTraceExt2( LONG_TRACE_TEST, ASCII300_ASCII512, "Ascii300: %s Ascii512: %s", buf300, buf512);
            }
        endTime = NKern::FastCounter();
        time = (endTime - startTime) * 1000 / freq;

        timeTestNumber5 = time;
        // ---------------------------- Ascii300 + Ascii512 ----------------------------            
 
        // ---------------------------- Ascii600 + Ascii300 ----------------------------       
        startTime = NKern::FastCounter();
        Kern::Printf( "--------------TraceCompilerTesterAdvanced Long Trace Test : Ascii600 + Ascii300----------------" ); 
        for (i=0; i < numberOfTraces; i++)
            {
            OstTraceExt2( LONG_TRACE_TEST, ASCII600_ASCII300, "Ascii600: %s Ascii300: %s", buf600, buf300);
            }
        endTime = NKern::FastCounter();
        time = (endTime - startTime) * 1000 / freq;

        timeTestNumber6 = time;
        // ---------------------------- Ascii600 + Ascii300 ----------------------------
        
        // ---------------------------- Ascii333 + Ascii333 ----------------------------       
        startTime = NKern::FastCounter();
        Kern::Printf( "--------------TraceCompilerTesterAdvanced Long Trace Test : Ascii333 + Ascii333----------------" ); 
        for (i=0; i < numberOfTraces; i++)
            {
            OstTraceExt2( LONG_TRACE_TEST, ASCII333_ASCII333, "Ascii333: %s Ascii333: %s", buf333, buf333);
            }
        endTime = NKern::FastCounter();
        time = (endTime - startTime) * 1000 / freq;

        timeTestNumber7 = time;
        // ---------------------------- Ascii333 + Ascii333 ----------------------------       
        
		Kern::Printf( "--------------TraceCompilerTesterAdvanced----------------" ); 
        Kern::Printf( "Ascii60: %d", timeTestNumber1 );
        Kern::Printf( "Ascii300: %d", timeTestNumber2 ); 
        Kern::Printf( "Ascii512: %d", timeTestNumber3 ); 
        Kern::Printf( "Ascii600: %d", timeTestNumber4 );
        Kern::Printf( "Ascii300 + Ascii512: %d", timeTestNumber5 );
        Kern::Printf( "Ascii600 + Ascii300: %d", timeTestNumber6 );
        Kern::Printf( "Ascii333 + Ascii333: %d", timeTestNumber7 );
		Kern::Printf( "--------------TraceCompilerTesterAdvanced----------------" );         
      
        Kern::Printf( "--------------TraceCompilerTesterAdvanced Long Trace Test END----------------" ); 
        }
    else if (aGroupId == SHORT_TRACE_TEST)
        {
        Kern::Printf( "--------------TraceCompilerTesterAdvanced Short Trace Test START----------------" ); 
        Kern::Printf( "--------------TraceCompilerTesterAdvanced Short Trace Test : Ascii1----------------" ); 
        OstTraceExt1( SHORT_TRACE_TEST, ASCII1, "Ascii1: %s", buf1);
        Kern::Printf( "--------------TraceCompilerTesterAdvanced Short Trace Test : Ascii2----------------" ); 
        OstTraceExt1( SHORT_TRACE_TEST, ASCII2, "Ascii2: %s", buf2);
        Kern::Printf( "--------------TraceCompilerTesterAdvanced Short Trace Test : Ascii3----------------" ); 
        OstTraceExt1( SHORT_TRACE_TEST, ASCII3, "Ascii3: %s", buf3);
        Kern::Printf( "--------------TraceCompilerTesterAdvanced Short Trace Test : Ascii4----------------" ); 
        OstTraceExt1( SHORT_TRACE_TEST, ASCII4, "Ascii4: %s", buf4);
        Kern::Printf( "--------------TraceCompilerTesterAdvanced Short Trace Test : Ascii1 + Acii2----------------" ); 
        OstTraceExt2( SHORT_TRACE_TEST, ASCII5, "Ascii1: %s Ascii2: %s", buf1, buf2);

        Kern::Printf( "--------------TraceCompilerTesterAdvanced Short Trace Test END----------------" ); 
        }
    else if (aGroupId == INCLUDED_NON_SOURCE_FILE_TEST)
        {
        // Print out traces from included non-source file that is defined in trace.properties file
        Kern::Printf( "--------------TraceCompilerTesterAdvanced Included Non-Source File Test START----------------" );
        
        this->HelloWorld();
        this->HelloMoon();
        
        TUint32 value3 = GetBigValue1000();
        TUint32 value4 = GetBigValue2000();
        
        Kern::Printf( "--------------TraceCompilerTesterAdvanced Included Non-Source File Test END----------------" );
                          
        }
    else if (aGroupId == TRACE_FLOW)
        {
        OstTrace0(TRACE_FLOW, TEST_TRACE_2, "Test OLD TraceCompiler");
        // This test can also use to test function parameters
        Kern::Printf( "--------------TraceCompilerTesterAdvanced Common Source File Test START----------------");
        DTraceCompilerTesterCommon1* common1 = new DTraceCompilerTesterCommon1();
        TInt32 sum = common1->SumValues(-1, -2, 100, 200);
        delete common1;        
        Kern::Printf( "--------------TraceCompilerTesterAdvanced Common Source File Test END----------------");
        }
    else if (aGroupId == KEKKONEN)
        {
        Kern::Printf( "--------------TraceCompilerTesterAdvanced Kekkonen START----------------");
        DTraceCompilerTesterCommon2* common2 = new DTraceCompilerTesterCommon2();
        common2->Kekkonen();
        delete common2;
        Kern::Printf( "--------------TraceCompilerTesterAdvanced Kekkonen END----------------");
        }
    else if (aGroupId == TRACE_STATE)
        {
        Kern::Printf( "--------------TraceCompilerTesterAdvanced State Trace Test START----------------");
        OstTraceState0(STATE_TRACE_0, "Kone1", "Running");
        OstTraceState1(STATE_TRACE_1, "Kone2", "Stopped", this);
        Kern::Printf( "--------------TraceCompilerTesterAdvanced  State Trace Test  END----------------");
        }
    else if (aGroupId == TRACE_DATA)
        {
        Kern::Printf( "--------------TraceCompilerTesterAdvanced Data Trace Test START----------------");
        
        TBuf8<601> data(buf600);
        OstTraceData( TRACE_DATA, TRACE_DATA_TRACE,
                    "DTraceCoreXtiIf::DebugPrintMsg - Msg via XTI IF 0x%{hex8[]}", data.Ptr(), data.Size() );
        Kern::Printf( "--------------TraceCompilerTesterAdvanced Data Trace Test  END----------------");
        }
    else if (aGroupId == POST_CODE_TEST)
        {
        Kern::Printf( "--------------TraceCompilerTesterAdvanced POST Code Trace Test START----------------");
        OstTrace0(POST_CODE_TEST, POST1, "Print some POST code traces");
        Kern::Printf("[POST][X-Loader][Started][OK]");
        Kern::Printf("[POST][BootUp][Reset reason][PowerOn]");
        Kern::NanoWait(50000000);
        Kern::Printf("[POST][X-Loader][Authenticate][OK]");
        Kern::NanoWait(100000000);
        Kern::Printf("[POST][X-Loader][load][OK]");
        Kern::NanoWait(100000000);
        Kern::Printf("[POST][X-Loader][Authenticate][OK]");
        Kern::NanoWait(700000000);
        Kern::Printf("[POST][X-Loader][load][OK]");
        Kern::NanoWait(50000000);
        Kern::Printf("[POST][X-Loader][load][OK]");
        Kern::NanoWait(50000000);
        Kern::Printf("[POST][X-Loader][load][OK]");
        Kern::NanoWait(50000000);
        Kern::Printf("[POST][X-Loader][Authenticate][OK]");
        Kern::NanoWait(200000000);
        Kern::Printf("[POST][Boot strap][Started][OK]");
        Kern::NanoWait(10000000);
        Kern::Printf("[POST][Estart][started][OK]");
        Kern::NanoWait(10000000);
        Kern::Printf("[POST][Estart][CompositeFileSys.][OK]");
        Kern::NanoWait(500000000);
        Kern::Printf("[POST][Mount][Drive][OK]");
        Kern::NanoWait(200000000);
        Kern::Printf("[POST][Mount][Drive][OK]");
        Kern::NanoWait(200000000);
        Kern::Printf("[POST][Estart][ISC API start][OK]");
        Kern::NanoWait(300000000);
        Kern::Printf("[POST][Estart][userdisk format][No]");
        Kern::NanoWait(10000000);
        Kern::Printf("[POST][Estart][secenv start][OK]");
        Kern::NanoWait(20000000);
        Kern::Printf("[POST][Estart][startup mode][Local]");
        Kern::NanoWait(10000000);
        Kern::Printf("[POST][Estart][Language selection][OK]");
        Kern::NanoWait(200000000);
        Kern::Printf("[POST][SelfTest][Started][FAIL][KErrOutOfMemory]");
        Kern::NanoWait(200000000);
        Kern::Printf("[POST][BT][Started][OK]");
        Kern::NanoWait(800000000);
        Kern::Printf("[POST][WLAN][Started][OK]");
        Kern::NanoWait(800000000);
        Kern::Printf("[POST][Display][Started][OK]");
        Kern::NanoWait(800000000);
        Kern::Printf("[POST][Camera][Started][OK]");
        Kern::NanoWait(80000000);
        Kern::Printf("[POST][mc_kernelagent][ExtensionInit0][OK]");
        Kern::NanoWait(100000000);
        Kern::Printf("[POST][mc_sender][Pre-init][OK]");
        Kern::NanoWait(100000000);
        Kern::Printf("[POST][mc_useragent][MobileCrashWriter init][OK]");        
        Kern::Printf( "--------------TraceCompilerTesterAdvanced POST Code Trace Test END----------------");
        }
    }
		
/**
 * Callback function for Trace Deactivation
 * 
 * @param aComponentId
 * @param aGroupId     
 */  
void DTraceCompilerTesterAdvanced::TraceDeactivated( TUint32 aComponentId, TUint16 aGroupId  )
    {
    Kern::Printf( "DTraceCompilerTesterAdvanced::TraceDeactivated - ComponentId = 0x%x, GroupId = 0x%x", aComponentId, aGroupId ); 
    }


/**
 * The entry point for a standard extension. Creates Long Trace Tester extension.
 *
 *  @return KErrNone, if successful
 */
DECLARE_STANDARD_EXTENSION()    //lint !e960 !e1717 ¤/#((& Symbian
    {
    
    TInt ret( KErrNone );
    
    DTraceCompilerTesterAdvanced* longTraceTester = new DTraceCompilerTesterAdvanced();

    if( longTraceTester == NULL )
        {
        ret = KErrNoMemory;
        }    
    
    if ( ret == KErrNone  )
        {
        // construct instance
        ret = longTraceTester->Construct();
        }
    
    return ret;
    }
// End of File
