/*
* Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description: This file contains testclass implementation.
*
*/

// [INCLUDE FILES] - do not remove
#include <e32svr.h>
#include <bautils.h>
#include <StifParser.h>
#include <StifTestInterface.h>
#include "PIEngineTest.h"

// EXTERNAL DATA STRUCTURES
//extern  ?external_data;

// EXTERNAL FUNCTION PROTOTYPES  
//extern ?external_function( ?arg_type,?arg_type );

// CONSTANTS
//const ?type ?constant_var = ?constant;

// MACROS
//#define ?macro ?macro_def

// LOCAL CONSTANTS AND MACROS
//const ?type ?constant_var = ?constant;
//#define ?macro_name ?macro_def

// MODULE DATA STRUCTURES
//enum ?declaration
//typedef ?declaration

// LOCAL FUNCTION PROTOTYPES
//?type ?function_name( ?arg_type, ?arg_type );

// FORWARD DECLARATIONS
//class ?FORWARD_CLASSNAME;

// ============================= LOCAL FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// ?function_name ?description.
// ?description
// Returns: ?value_1: ?description
//          ?value_n: ?description_line1
//                    ?description_line2
// -----------------------------------------------------------------------------
//
/*
?type ?function_name(
    ?arg_type arg,  // ?description
    ?arg_type arg)  // ?description
    {

    ?code  // ?comment

    // ?comment
    ?code
    }
*/

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CPIEngineTest::Delete
// Delete here all resources allocated and opened from test methods. 
// Called from destructor. 
// -----------------------------------------------------------------------------
//
void CPIEngineTest::Delete() 
    {

    }

// -----------------------------------------------------------------------------
// CPIEngineTest::RunMethodL
// Run specified method. Contains also table of test mothods and their names.
// -----------------------------------------------------------------------------
//
TInt CPIEngineTest::RunMethodL( 
    CStifItemParser& aItem ) 
    {

    static TStifFunctionInfo const KFunctions[] =
        {  
        // Copy this line for every implemented function.
        // First string is the function name used in TestScripter script file.
        // Second is the actual implementation member function. 
        ENTRY( "StartEngineAndSample10Sec", CPIEngineTest::StartEngineAndSample10SecL ),
        //ADD NEW ENTRY HERE
        // [test cases entries] - Do not remove

        };

    const TInt count = sizeof( KFunctions ) / 
                        sizeof( TStifFunctionInfo );

    return RunInternalL( KFunctions, count, aItem );

    }

TInt CPIEngineTest::CPIEngineTest::StartEngineAndSample10SecL( CStifItemParser& aItem )
    {
    // Print to UI
    _LIT( KPIEngineTest, "PIEngineTest" );
    _LIT( KPIEngineExeFile,"Z:\\sys\\bin\\PIProfiler.exe" );
    _LIT( KPIEngineExeParameters,"timed Z:\\data\\PIProfilerSettings.txt 10" );
    _LIT( KLogFileName, "C:\\data\\PIProfilerTest_1.dat" );
    
    TestModuleIf().Printf( 0, KPIEngineTest, _L("Check for existing data log files") );  
    iLog->Log( _L("Check for existing data log files") );

    // Remove existing test data log file if it any. 
    RFs fsSession;
    User::LeaveIfError(fsSession.Connect());
    if ( BaflUtils::FileExists(fsSession, KLogFileName) )
        {
        TestModuleIf().Printf( 0, KPIEngineTest, _L("Removing existing file") );  
        iLog->Log( _L("Removing existing file") );
                
        User::LeaveIfError(fsSession.Delete(KLogFileName));
        }

    fsSession.Close();

    TestModuleIf().Printf( 0, KPIEngineTest, _L("Start profiling for 10 seconds") );
    iLog->Log(_L("Start profiling for 10 seconds"));

    // Create engine process with timed profiling parameter and setting file.
    RProcess proc;
    TInt status = proc.Create(KPIEngineExeFile, KPIEngineExeParameters);

    // Check if creating process failed
    if(status != KErrNone) 
        {
        TestModuleIf().Printf( 0, KPIEngineTest, _L("Failed to start profiling") );
        iLog->Log( _L("Failed to start profiling") );
                
        return status;
        }

    TestModuleIf().Printf( 0, KPIEngineTest, _L("profiling...") );
    iLog->Log( _L("profiling...") );
            
    proc.Resume();

    // Wait for process to finish
    User::After(15000000);

    if (proc.ExitReason() != KErrNone)
        {
        TestModuleIf().Printf( 0, KPIEngineTest, _L("Process exited with reason: %d"), proc.ExitReason());
        iLog->Log( _L("Process exited with reason: %d"), proc.ExitReason() );

        proc.Close();

        return proc.ExitReason();
        }

    proc.Close();

    TInt err = KErrNone;
    User::LeaveIfError(fsSession.Connect());
    
    // Check if data log file was succesfully created and remove it
    if ( BaflUtils::FileExists(fsSession, KLogFileName) )
        {
        TestModuleIf().Printf( 0, KPIEngineTest, _L("Data log file was succesfully created") );
        iLog->Log( _L("Data log file was succesfully created") );
        
        fsSession.Delete(KLogFileName);
        }
    else
        {
        TestModuleIf().Printf( 0, KPIEngineTest, _L("Data log file was not created") );
        iLog->Log( _L("Data log file was not created") );
                
        err = KErrPathNotFound;
        }
    
    fsSession.Close();
    return err;
    }

// -----------------------------------------------------------------------------
// CPIEngineTest::?member_function
// ?implementation_description
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
/*
TInt CPIEngineTest::?member_function(
   CItemParser& aItem )
   {

   ?code

   }
*/

// ========================== OTHER EXPORTED FUNCTIONS =========================
// None

//  [End of File] - Do not remove

// End of File
