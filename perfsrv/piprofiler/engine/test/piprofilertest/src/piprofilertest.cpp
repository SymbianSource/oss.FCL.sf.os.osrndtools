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
* Description:  
*
*/

//  Include Files  

#include "piprofilertest.h"
#include <e32base.h>
#include <e32std.h>
#include <e32cons.h>			// Console
#include <e32debug.h>
#include "rtestsampler.h"

//  Constants

_LIT(KTextConsoleTitle, "Console");
_LIT(KTextFailed, " failed, leave code = %d");
_LIT(KTextPressAnyKey, " [press any key]\n");

//  Global Variables

LOCAL_D CConsoleBase* console; // write all messages to this


//  Local Functions

LOCAL_C void MainL()
    {
    //
    // add your program code here, example code below
    //
    //console->Write(_L("Hello, world!\n"));
    RDebug::Printf("hallo welt\n");
    //LoadProfilerLDD();
    
    RDebug::Printf("hallo ldd\n");
    LoadProfilerDLL();
    RDebug::Printf("hallo2\n");
    }

LOCAL_C void DoStartL()
    {
    // Create active scheduler (to run active objects)
    CActiveScheduler* scheduler = new (ELeave) CActiveScheduler();
    CleanupStack::PushL(scheduler);
    CActiveScheduler::Install(scheduler);

    MainL();
    
    // Delete active scheduler
    CleanupStack::PopAndDestroy(scheduler);
    }

//  Global Functions

GLDEF_C TInt E32Main()
    {
    // Create cleanup stack
    __UHEAP_MARK;
    CTrapCleanup* cleanup = CTrapCleanup::New();
    // Create output console
//    TRAPD(createError, console = Console::NewL(KTextConsoleTitle, TSize(
//            KConsFullScreen, KConsFullScreen)));
//    if (createError)
//        {
//        delete cleanup;
//        return createError;
//        }

    // Run application code inside TRAP harness, wait keypress when terminated
    TRAPD(mainError, DoStartL());
    if (mainError)
//        console->Printf(KTextFailed, mainError);
//    console->Printf(KTextPressAnyKey);
//    console->Getch();

//    delete console;
    delete cleanup;
    __UHEAP_MARKEND;
    return KErrNone;
    }

LOCAL_C TInt LoadProfilerLDD()
   {
    RDebug::Printf("LoadProfilerLDD >");
    TInt err;        
    _LIT(KPluginSamplerName,"PIProfilerGeneralsSampler");
    err = User::LoadLogicalDevice(KPluginSamplerName);
    RDebug::Printf("Err2 : %d", err);
    if(err == KErrAlreadyExists)
        {
        err = User::FreeLogicalDevice(KPluginSamplerName);
        RDebug::Printf("Err2.1 : %d", err);
        err = User::LoadLogicalDevice(KPluginSamplerName);
        RDebug::Printf("Err2.2 : %d", err);
        }
    RtestSampler aGeneralsSampler;
    err = aGeneralsSampler.Open();
    RDebug::Printf("Err3 : %d", err);
    if(err = KErrNone)
        {
        RDebug::Printf("test - Could not open sampler device: %d", err);
        // Sets test case result and description(Maximum size is KStifMaxResultDes)
        _LIT( KDescription, "Could not open sampler device" );
        //aResult.SetResult( KErrNone, KDescription );
        }
    
    // Case was executed
    err = User::FreeLogicalDevice(KPluginSamplerName);
    RDebug::Printf("Err4 : %d", err);
    
    RDebug::Printf("LoadProfilerLDD <");
    return KErrNone;
   }

LOCAL_C TInt LoadProfilerDLL()
   {    
        RDebug::Printf("LoadProfilerDLL >");
        RLibrary lib;
        TInt err = lib.Load(_L("PIProfilerGenerals.dll"), _L("c:\\sys\\bin"));
        RDebug::Printf("Err0 : %d", err);
        
        const TInt KNewLOrdinal = 2;
        TLibraryFunction NewL =lib.Lookup(KNewLOrdinal);                
        RDebug::Printf("library.lookup returns 0x%x", NewL);
        CSamplerPluginInterface* mydll;
        //STF_ASSERT_LEAVES_WITH (KErrPermissionDenied, (mydll=(CSamplerPluginInterface*)NewL()));
        mydll=(CSamplerPluginInterface*)NewL();
        
        if( mydll )
            {
                // Sets test case result and description(Maximum size is KStifMaxResultDes)
                _LIT( KDescription, "mydll exists" );
                //aResult.SetResult( KErrAccessDenied, KDescription );

            }
        else
            {
                // Sets test case result and description(Maximum size is KStifMaxResultDes)
                _LIT( KDescription, "passed" );
                //aResult.SetResult( KErrNone, KDescription );
            }
        
        // Case was executed
        RDebug::Printf("LoadProfilerDLL <");
        return KErrNone;
   }

