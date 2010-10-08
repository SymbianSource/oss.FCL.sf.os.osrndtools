// Copyright (c) 2005-2010 Nokia Corporation and/or its subsidiary(-ies).
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
// t_tracecore_wdp.cpp
// Overview:
// Tests WDP TraceCore 
// kernel - side APIs
//

#define __E32TEST_EXTENSION__
#include <e32std.h>
#include <dptest.h>
#include "t_tracecore_wdp.h"
#include "t_rtest_panic.h"


RTest test(_L("T_TRACECORE_WDP"));

TInt CreateProcess (const TDesC &aProcessName, const TDesC &aCommand, RProcess &aProcess)
    {
    TInt err = aProcess.Create(aProcessName,aCommand);
    
    if (err != KErrNone)
        test.Printf(_L("Unable to create process handle. Error: %d"),err);
    
    return err;
    }

void StartProcess (RProcess &aProcess, TRequestStatus &aStatus)
    {
    aProcess.Logon(aStatus);
    aProcess.Resume();
    }

TInt CreateAndStartProcess (const TDesC &aProcessName, const TDesC &aCommand, RProcess &aProcess, TRequestStatus &aStatus)
    {
    TInt err = CreateProcess(aProcessName,aCommand,aProcess);
    if (err != KErrNone)
        return err;
    
    StartProcess(aProcess,aStatus);
    return KErrNone;
    }

TInt ContinueFlushingPageCache(TRequestStatus &aTestStatus)
    {
    while (aTestStatus==KRequestPending)
        {
        //Flush the cache
        DPTest::FlushCache();
        //wait for 100 microseconds
        User::After(100);
        }
    return KErrNone;
    }

TInt CheckExecutableReturn(const TDesC &aProcessName, RProcess &aTestProcess)
    {
    TInt exitreason = aTestProcess.ExitReason();
    TPtrC exitcategory(KExitCategoryUser);
    
    //if the test didn't complete with error, just return KErrNone
    if (exitreason == KErrNone) 
        return exitreason;
    
    //most of this next bit will not be needed tests pass
    
    //if it completed with error ensure it wasn't t_tracecore.exe because
    //that test should pass. If it was t_tracecore.exe return the exit reason
    //and fail the test.
        if (((aProcessName.Compare(KTTraceCore))==0)||((aProcessName.Compare(KTRTraceBuffer))==0)||((aProcessName.Compare(KTOstBuffer))==0)
		||((aProcessName.Compare(KTOstBufferStress))==0)||((aProcessName.Compare(KTMultipart))==0)||((aProcessName.Compare(KTTraceCoreOstLdd))==0))
        {
        test.Printf(_L("Test completed with error where it shouldn't: Category: %S, Code: %d"),&exitcategory,exitreason);
        return exitreason;
        }
    
    //if it was any of the other executables i.e t_tracecoreostldd.exe 
    //or TraceCoreTestApp.exe that completed with error check it's USER 84
    //because that is what is expected at the moment. If that wasn't the panic
    //return the exit reason and fail the test. Else return KErrNone.
    if ((exitreason==84)&&((exitcategory.Compare(KExitCategoryUser))==0))
        return KErrNone;
    else
        {
        test.Printf(_L("Test completed with unknown error: Category: %S, Code: %d"),&exitcategory,exitreason);
        }
    return exitreason;
    }

TInt RunTestExecutable(const TDesC &aProcessName)
    {
    TInt err = KErrNone;
    RProcess testprocess;
    TRequestStatus teststatus;
    
    //Create and start the test process
    err = CreateAndStartProcess(aProcessName,KNullCmd,testprocess,teststatus);
    if (err!=KErrNone)
        {
        testprocess.Close();
        return err;
        }
    
    //Flush Cache every 100us until test finishes running
    err = ContinueFlushingPageCache(teststatus);
    if (err!=KErrNone)
        {
        testprocess.Close();
        return err;
        }
    
    //Check return from test executable
    err = CheckExecutableReturn(aProcessName,testprocess);
    if (err!=KErrNone)
        {
        TExitCategoryName exitCategory = testprocess.ExitCategory();
        testprocess.Close();
        if ((err==84)&&((exitCategory.Compare(KExitCategoryUser))==0))
            {
            // If process exited with RTest panic, then use process name as panic category
            User::Panic(aProcessName, err);
            }
        else
            {
            // If process exited with any other panic, use that as the panic category
            User::Panic(exitCategory, err);
            }
        }
    
    return err;
    }

GLDEF_C TInt E32Main()
    {

    test.Title();
    test.Start(_L("Trace Core WDP tests"));
    TInt err = KErrNone;
    
    test.Next(_L("Run t_tracecore.exe while repeatedly flushing the page cache"));
    err = RunTestExecutable(KTTraceCore);
    TEST_KErrNone(err);
    
    test.Next(_L("Run t_ostbuffer_stress.exe while repeatedly flushing the page cache"));
    err = RunTestExecutable(KTOstBufferStress);
    TEST_KErrNone(err);
    
    test.Next(_L("Run t_multipart.exe while repeatedly flushing the page cache"));
    err = RunTestExecutable(KTMultipart);
    TEST_KErrNone(err);
    
    test.Next(_L("Run t_rtracebuffer.exe while repeatedly flushing the page cache"));
    err = RunTestExecutable(KTRTraceBuffer);
    TEST_KErrNone(err);
    
    test.Next(_L("Run t_ostbuffer.exe while repeatedly flushing the page cache"));
    err = RunTestExecutable(KTOstBuffer);
    TEST_KErrNone(err);
    
//    test.Next(_L("Run t_tracecoreostldd.exe while repeatedly flushing the page cache"));
//    err = RunTestExecutable(KTTraceCoreOstLdd);
//    TEST_KErrNone(err);
//    
//    test.Next(_L("Run TraceCoreTestApp.exe while repeatedly flushing the page cache"));
//    err = RunTestExecutable(KTraceCoreTestApp);
//    TEST_KErrNone(err);

    
    test.Printf(_L("\nFinished Trace Core WDP tests!!!"));

    test.End();
    test.Close();
    return (0);
    }
