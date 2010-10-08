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
// e32test\ost\t_tracecore.cpp
// Overview:
// Tests activation/deactivation of traces using the TraceCore 
// kernel - side APIs
//

#define __E32TEST_EXTENSION__
#include <e32svr.h>
#include <e32def.h>
#include <e32def_private.h>
#include <e32btrace.h>
#include <test/tracedataparser.h>
#include "t_rtest_panic.h"


// the following definition is mutually exclusive
// with loading tracecore as extension 
// eg epoc.ini extension tracecore.dll


#include "TraceCoreConstants.h"
#include "d_tracecore.h"
#include "t_tracecore.h"
#include "TestDataWriterNotifier.h"

#define __TRACE_LINE__()    test.Printf(_L("%d\n"),__LINE__)

RTest test(_L("T_TRACECORE"));

RTraceCoreTest TraceTest;
TComponentId KOstTraceComponentID;
TUint32 KTraceMultipartTestData[60];
TBuf8<KTcTdwBufSize> buf;



/**
 * Validate all filters are in sync for the component ids in this test
 * @param aComponentId the component id to check group id (category) filters on 
 */
void ValidateGroupIdFiltersInSync(TUint32 aComponentId)
    {
    TInt r = KErrNone;
    TcDriverParameters p;
    
    p.iComponentId = aComponentId;
    for (TInt i = BTrace::EThreadIdentification; i < KMaxGroupId; i++)
        {
        p.iGroupId = i;
        r = TraceTest.ValidateFilterSync(p);
        TEST_KErrNone(r);
        }
    }

void ValidateAllFiltersInSync()
    {
    ValidateGroupIdFiltersInSync(KTestComponentID1);
    ValidateGroupIdFiltersInSync(KTestComponentID2);
    }

/*
 * Activate a trace depending on aMatch
 * 
 * @param aMatch if aMatch is true then activate the correct GID  and CID
 * @param aNum Number of group id's and component id pairs to activate
 * 
 */
void ActivateTrace(TBool aMatch, TInt aNum = 1)
    {
    test.Printf(_L("In ActivateTrace()\n"));
    TcDriverParameters p;
    TInt r = 0;
    p.iComponentId = (aMatch) ? KTestComponentID1 : KTestComponentID2;
    p.iGroupId = (aMatch) ? KTestGroupId1 : KTestGroupId2;

    r = TraceTest.ValidateFilterSync(p);
    TEST_KErrNone(r);

    //if we want to activate correct gids &cids, activate the correct number of them
    //starting from KTestGroupId1 and KTestComponentID1 else just activate one incorrect 
    //pair i.e KTestComponentID2 & KTestGroupId1
    if (aMatch)
        r = TraceTest.ActivateTrace(p, aNum);
    else
        r = TraceTest.ActivateTrace(p);
    TEST_KErrNone(r);

    r = TraceTest.ValidateFilterSync(p);
    TEST_KErrNone(r);

    }

/*
 * Deactivate a trace depending on aMatch
 * 
 * @param aMatch if aMatch is true then deactivate the correct GID  and CID
 * @param aNum Number of group id's to deactivate
 * 
 */
void DeactivateTrace(TBool aMatch, TInt aNum = 1)
    {
    test.Printf(_L("In DeactivateTrace()\n"));
    TcDriverParameters p;
    TInt r = 0;
    p.iComponentId = (aMatch) ? KTestComponentID1 : KTestComponentID2;
    p.iGroupId = (aMatch) ? KTestGroupId1 : KTestGroupId2;

    r = TraceTest.ValidateFilterSync(p);
    TEST_KErrNone(r);

    //if we want to deactivate correct gids &cids, deactivate the correct number of them
    //starting from KTestGroupId1 and KTestComponentID1 else just deactivate one incorrect 
    //pair i.e KTestComponentID2 & KTestGroupId1  
    if (aMatch)
        r = TraceTest.DeactivateTrace(p, aNum);
    else
        r = TraceTest.DeactivateTrace(p);
    TEST_KErrNone(r);

    r = TraceTest.ValidateFilterSync(p);
    TEST_KErrNone(r);
    }

/*
 * Tell the test writer to start/stop dropping
 * traces
 * 
 * @param aDrop ETrue to drop Traces, EFalse to stop
 *              dropping them
 */
void DropNextTrace(TBool aDrop)
    {
    TInt r = KErrNone;
    r = TraceTest.DropNextTrace(aDrop);
    TEST_KErrNone(r);
    }

/*
 * Activate/deactivate the printf handler
 *
 * TBool aActivate to activate(ETrue)/deactivate(EFalse)
 *
 * Activate/Deactivate    TRACECORE_KERN_PRINTF=BTrace::EKernPrintf=0
 *                        TRACECORE_RDEBUG_PRINT=BTrace::ERDebugPrintf=1
 *                        TRACECORE_PLATSEC_PRINTF=BTrace::EPlatsecPrintf=2
 *
 */
void ActivatePrintfHandler(TBool aActivate, TInt aMaxCategory = BTrace::EPlatsecPrintf)
    {
    TcDriverParameters p;
    TInt r=0;
    TInt maxPrintfCategory = aMaxCategory;
    p.iComponentId = 0x2001022D; //from BTraceHooks_0x2001022D_dictionary.xml
    p.iGroupId = BTrace::ERDebugPrintf;

    for(TInt i=0; i<=maxPrintfCategory; i++)
        {
        if(aActivate)
            r = TraceTest.ActivateTrace(p);
        else
            r = TraceTest.DeactivateTrace(p);
        if (r!=KErrNone)
            {
            test.Printf(_L("Unable to activate/deactivate printf handler for printf %d: error: %d"), i, r);
            }
        TEST_KErrNone(r);
        p.iGroupId++;
        }
    r = TraceTest.ValidateFilterSync(p);
    TEST_KErrNone(r);
    }

/*
 * Activate/deactivate kernel GIDs
 *
 * TBool aActivate to activate(ETrue)/deactivate(EFalse)
 * TBool aFirstGID start of range of GIDs to activate/deactivate
 * TBool aLastGID end of range of GIDs to activate/deactivate
 *
 */
void ActivateKernelTraces(TBool aActivate, TInt aFirstGID = -1, TInt aLastGID = -1)
    {
    TInt r=KErrNone;

    if(aFirstGID == -1)
        {
        aFirstGID = KMinKernelCategory;
        if(aLastGID == -1)
            {
            aLastGID = KMaxKernelCategory;
            }
        }
    if(aLastGID < aFirstGID)
        {
        aLastGID = aFirstGID;
        }
        
    TcDriverParameters p;
    p.iComponentId = 0x2001022D; //from BTraceHooks_0x2001022D_dictionary.xml

    for(TInt i=aFirstGID; i<=aLastGID; i++)
        {
        p.iGroupId = i;
        if(aActivate)
            r = TraceTest.ActivateTrace(p);
        else
            r = TraceTest.DeactivateTrace(p);
        if (r<KErrNone)
            {
            test.Printf(_L("Unable to activate/deactivate kernel trace for GID %d: error: %d"), i, r);
            }
        TEST_KErrNone(r);
        }
    }

/*
 * Validate the trace data in memory buffer
 * 
 * @param aTracePresent determines if trace data should be present or not
 * @param aBuffer the buffer containing the trace data
 * @param aSize size of the trace buffer
 * @param aGID expected GID
 * @param aCID expected CID
 * 
 */
TInt ValidatePayload(TBool aTracePresent, TUint8 aGID, TUint32 aCID, TBool aMissingTrace=EFalse, TBool aPrintfTrace=EFalse, TDesC8* aExpectedPrintfTrace=NULL)
    {
#ifdef TRACECORE_TRACE_DISABLED
    if (!aPrintfTrace)
        {
        aTracePresent = EFalse;
        }
#endif
    return TTraceDataParser::ValidatePayload(buf, aTracePresent, 
            (TGroupId) aGID, (TComponentId)aCID, KTestData, aPrintfTrace, aMissingTrace, aExpectedPrintfTrace);
    }

/*
 * Make a request to the test writer to start watching for data
 * 
 * @param aStatus holds the status of the request
 * @param aNumTraces number of traces to write
 * 
 */
void RequestDataNotification(TRequestStatus& aStatus, TInt aNumTraces=1)
    {
    buf.Zero();
    TraceTest.RequestTraceData(aStatus, buf, aNumTraces);
    }

/*
 * Read the trace data in memory buffer
 * 
 * @param aBuffer on return, buffer containing the trace data
 * @param aSize on return, size of the trace buffer
 * @return
 * 
 */
TInt ReadTraceFromBuffer(TRequestStatus& aStatus, TUint8*& aBuffer,
        TInt& aSize)
    {
    RTimer timer;
    TRequestStatus status;
    TInt ret = timer.CreateLocal();
    // wait for WriteComplete to complete

    timer.After(status, 10000);
    User::WaitForRequest(status,aStatus);

    if (aStatus.Int() != KErrNone)
        //if the read request didn't complete, force it to look like it completed (this doesn't cancel the request tho)
        aStatus = KErrNone;

    aBuffer = (TUint8*) buf.Ptr();
    aSize = buf.Size();

    //test.Printf(_L("In ReadTraceFromBuffer()\n"));
    timer.Close();
    return KErrNone;
    }

/*
 * Send Simple (<80 bytes) Test Traces. 
 * Each trace has the same payload containing numbers
 * starting from KTestData and incrementing until the required size is filled up.
 * Each trace has a different group id and component id pair incrementing up from
 * KTestGroupId1 and KTestComponentID1.
 * 
 * @param aTracePresent indicates id a trace packet should be present or not
 * @param aSize size of trace to send in words (groups of 4 bytes),defaults to 1.
 * @param aNum Number of traces to send, defaults to 1.
 */
TInt SendAndValidateTestTraces(TBool aTracePresent, TInt aSize = 1,
        TInt aNum = 1, TBool aPrintfTrace = EFalse, TInt aNumberDropped = 0)
    {
    if (aSize > 18)
        {
        test.Printf(
                _L("Incorrect argument... Please use a trace size that is <=18 words (72bytes)"));
        return KErrArgument;
        }
    
    if (aNumberDropped>=aNum)
        {
        test.Printf(
                _L("Incorrect argument... Please use aNumberDropped less than number you wish to send)"));
        return KErrArgument;
        }
    
    test.Printf(_L("In SendTestTraces()\n"));
    TUint32 traceword = TEST_TRACE;
    KOstTraceComponentID = KTestComponentID1;
    TRequestStatus status;
    TUint8* buffer = NULL;

    //Drop next Trace
    DropNextTrace(ETrue);
    TBool dropped = ETrue;
    TBool missing = EFalse;
    
    for (TInt i = 0; i < aNum; i++)
        {
        if (i>=aNumberDropped)
            {
            DropNextTrace(EFalse); //stop dropping traces
            dropped = EFalse;
            }
        
        if ((i==aNumberDropped)&&(i!=0))
            missing = ETrue;
        else
            missing = EFalse;
        
        TInt size = 0;
        
        //make request to read trace data from memory
        TInt bufferedTraces=1;
        if (aPrintfTrace&&missing&&!dropped)              // if it's a printf trace and we're dropped traces
            bufferedTraces++;                             // and the one we're currently sending isn't to be dropped
        RequestDataNotification(status, bufferedTraces);
        
        if (!aPrintfTrace)
            OstTraceData(TRACE_NORMAL+i, traceword, "You will only see this text in Trace Viewer: %d", KTraceMultipartTestData,(aSize*4));
        else
            {
            ActivatePrintfHandler(ETrue,BTrace::ERDebugPrintf);
            RDebug::Printf(KTestPrintfTraceString);
            ActivatePrintfHandler(EFalse,BTrace::ERDebugPrintf);
            }
            
        //read traces from memory
        TEST_KErrNone(ReadTraceFromBuffer(status, buffer, size));

        //Validate Traces
        //test.Next(_L("Validate payload"));
        TPtrC8 expectedPrintfData(KTestPrintfTraceLiteral);
        TInt r = ValidatePayload((aTracePresent&&!dropped), TRACE_NORMAL+i, KOstTraceComponentID, missing, aPrintfTrace, &expectedPrintfData);
        TEST_KErrNone(r);
        traceword += (1 << GROUPIDSHIFT); //incrememnt the group id part of the trace word...
        KOstTraceComponentID++;
        }

    return KErrNone;
    }

/*
 * Send a big (>80bytes)Test Trace
 * 
 * @param aTracePresent indicates id a trace packet should be present or not
 */
void SendAndValidateBigTestTrace(TBool aTracePresent)
    {
    test.Printf(_L("In SendBigTestTrace()\n"));
    KOstTraceComponentID = KTestComponentID1;
    TRequestStatus status;

    TUint8* buffer = NULL;
    TInt size = 0;

    //make request to read trace data from memory
    RequestDataNotification(status);

    OstTraceData(TRACE_NORMAL, TEST_TRACE, "You will only see this text in Trace Viewer", KTraceMultipartTestData,240);
    TEST_KErrNone(ReadTraceFromBuffer(status, buffer, size));

    //Validate Traces
    test.Next(_L("Validate payload for test big trace"));
    TEST_KErrNone(ValidatePayload(aTracePresent,TRACE_NORMAL,KOstTraceComponentID));
    }

/*
 * Test Trace Activation in Trace core
 * 
 * @pre Trace Core settings must be cleared before trying to activate correct attributes
 */
void TestBasicActivation()
    {
    test.Printf(_L("In TestBasicActivation()\n"));

    //Send Activation
    test.Next(_L("Send Activation message to TraceCore"));
    ActivateTrace(ETrue);

    //Send and Validate Traces
    TEST_KErrNone(SendAndValidateTestTraces(ETrue));

    //Deactivate when done sending trace
    DeactivateTrace(ETrue);
    }

/*
 * Test Trace Deactivation in Trace core
 * 
 */
void TestBasicDeactivation()
    {
    test.Printf(_L("In TestBasicDeactivation()\n"));

    //Have correct trace attributes activated, then try to deactivate them
    ActivateTrace(ETrue);

    //Send Deactivation
    test.Next(_L("Send Deactivation message to TraceCore"));
    DeactivateTrace(ETrue);

    //Send and Validate Traces
    TEST_KErrNone(SendAndValidateTestTraces(EFalse));
    }

/*
 * Negative Test Trace Activation in Trace core
 * 
 * @pre Trace Core settings must be cleared before trying to activate wrong attributes
 */
void NegativeTestActivation()
    {
    test.Printf(_L("In NegativeTestActivation()\n"));

    //Send Activation
    test.Next(_L("Send Negative Activation message to TraceCore"));
    ActivateTrace(EFalse);

    //Send and Validate Traces
    TEST_KErrNone(SendAndValidateTestTraces(EFalse));

    //Deactivate when done sending trace
    DeactivateTrace(EFalse);
    }

/*
 * Test Trace Deactivation in Trace core
 * 
 */
void NegativeTestDeactivation()
    {
    test.Printf(_L("In NegativeTestDeactivation()\n"));

    //Have correct trace attributes activated, then try to deactivate the wrong ones
    ActivateTrace(ETrue);

    //Send Deactivation
    test.Next(_L("Send Negative Deactivation message to TraceCore"));
    DeactivateTrace(EFalse);

    //Send and Validate Traces
    TEST_KErrNone(SendAndValidateTestTraces(ETrue));

    //Deactivate when done sending trace
    DeactivateTrace(ETrue);
    }

/*
 * Test Trace Activation in Trace core when multiple (16 here) GID and CID pairs are activated
 * 
 * @pre Trace Core settings must be cleared before trying to activate correct attributes
 */
void TestMultipleActivation()
    {
    test.Printf(_L("In TestMultipleActivation()\n"));

    //Send Activation
    test.Next(_L("Send Activation message to TraceCore"));
    ActivateTrace(ETrue, 16);

    //Send and Validate Traces
    TEST_KErrNone(SendAndValidateTestTraces(ETrue,1,16));

    //Deactivate when done sending trace
    DeactivateTrace(ETrue, 16);
    }

/*
 * Test Trace Deactivation in Trace core when multiple (64 here) GID and CID pairs are activated
 * 
 * @pre Trace Core settings must be cleared before trying to activate correct attributes
 */
void TestMultipleDeactivation()
    {
    test.Printf(_L("In TestMultipleDeactivation()\n"));

    //Have correct trace attributes activated, then try to deactivate them
    ActivateTrace(ETrue, 16);

    //Send Deactivation
    test.Next(_L("Send Deactivation message to TraceCore"));
    DeactivateTrace(ETrue, 16);

    //Send Traces
    TEST_KErrNone(SendAndValidateTestTraces(EFalse,1,16));
    }

/*
 * Test Trace Activation when big traces are sent
 * 
 */
void TestBigTrace()
    {
    test.Printf(_L("In TestBigTrace()\n"));

    //Send Activation
    test.Next(_L("Send Activation message to TraceCore"));
    ActivateTrace(ETrue);

    //Send and Validate Big Trace
    SendAndValidateBigTestTrace(ETrue);

    //Deactivate when done sending trace
    DeactivateTrace(ETrue);

    }

/*
 * Test that TraceCore Handlers ouput missing data notification when traces are dropped
 */
void TestDroppedTraces()
    {
    test.Printf(_L("In TestDroppedTraces()\n"));

    //Send Activation
    test.Next(_L("Send Activation message to TraceCore"));
    ActivateTrace(ETrue, 5);
    
    //Send and Validate 5 Traces when no traces are dropped
    TEST_KErrNone(SendAndValidateTestTraces(ETrue,1,5,EFalse,0));

    //Send and Validate 5 Traces where 1st 2 traces are dropped (check for missing flag on 3rd)
    TEST_KErrNone(SendAndValidateTestTraces(ETrue,1,5,EFalse,2));

    //Send and Validate 1 Printf Trace where no traces are dropped
    TEST_KErrNone(SendAndValidateTestTraces(ETrue,1,5,ETrue,0));
    
    //Send and Validate 2 Printf Trace where 1st trace is dropped
    TEST_KErrNone(SendAndValidateTestTraces(ETrue,1,2,ETrue,1));//(check for "dropped trace" on 2nd)
    
    //Send and Validate 7 Printf Trace where 1st 5 traces are dropped
    TEST_KErrNone(SendAndValidateTestTraces(ETrue,1,7,ETrue,5));//(check for "dropped trace" on 6th)
    
    //Deactivate when done sending traces
    DeactivateTrace(ETrue, 5);
    }

/*
 * Test that when activation notification is received, the group is actually activated from TraceCore
 */
void TestActivationNotification()
    {
    test.Printf(_L("In TestActivationNotification()\n"));
    
    // Create activation parameters
    TcDriverParameters p; p.iComponentId = 5; p.iGroupId = 7;
    TcDriverParameters p2; p2.iComponentId = 6; p2.iGroupId = 8;
    
    // Register activation notification listener
    TraceTest.RegisterActivationNotification(p, ETrue);
    
    // Activate and test that we receive notification and it's in sync with TraceCore
    for (TInt i=0;i<5;i++)
        {
        TraceTest.ActivateTrace(p);
        TEST_KErrNone(TraceTest.CheckActivationNotificationOk(ETrue));
        TraceTest.DeactivateTrace(p);
        TEST_KErrNone(TraceTest.CheckActivationNotificationOk(ETrue));
        }
    
    // Activate with different ID's, we shouldn't get notification
    TraceTest.ActivateTrace(p2);
    TEST_KErrNone(TraceTest.CheckActivationNotificationOk(EFalse));
    
    // Unregister activation notification listener
    TraceTest.RegisterActivationNotification(p, EFalse);
    }

/*
 * Test RefreshActivations() method for kernel GIDs
 * 
 * The purpose of this test is to make sure TraceCore can refresh all activations with no error when
 * different numbers of kernel GIDs have been activated
 */
void TestRefreshActivations()
    {
    test.Printf(_L("In TestRefreshActivations()\n"));

    test.Next(_L("Test RefreshActivations() when all kernel GIDs are deactivated\n"));

    //Deactivate all kernel GIDs
    ActivateKernelTraces(EFalse);
    
    //deactivate and reactivate all active GIDs
    TEST_KErrNone(TraceTest.RefreshActivations());    
    
    test.Next(_L("Test RefreshActivations() when all some kernel GIDs are activated\n"));

    //Activate some kernel GIDs
    ActivateKernelTraces(ETrue,BTrace::EThreadIdentification);
    ActivateKernelTraces(ETrue,BTrace::EPaging);
    ActivateKernelTraces(ETrue,BTrace::EClientServer);
    
    //deactivate and reactivate all active GIDs
    TEST_KErrNone(TraceTest.RefreshActivations());    

    //Deactivate the kernel GIDs previously activated
    ActivateKernelTraces(EFalse,BTrace::EThreadIdentification);
    ActivateKernelTraces(EFalse,BTrace::EPaging);
    ActivateKernelTraces(EFalse,BTrace::EClientServer);

    test.Next(_L("Test RefreshActivations() when all kernel GIDs are activated\n"));

    //Activate all kernel GIDs
    ActivateKernelTraces(ETrue);

    //deactivate and reactivate all active GIDs
    TEST_KErrNone(TraceTest.RefreshActivations());    

    //Deactivate all kernel GIDs
    ActivateKernelTraces(EFalse);
    }

/*
 * Utility function for generating a trace packet 
 * until the test memory writer is available
 * 
 */
void GenerateTraceData()
    {

    //actual data that will be sent in trace tests    
    for (TInt i = 0; i < 60; i++)
        KTraceMultipartTestData[i] = KTestData + i;
    }

GLDEF_C TInt E32Main()
    {

    test.Title();

    test.Start(_L("Trace Core tests"));

    GenerateTraceData();

    // free the tcore related LDD's this is because 
    // if one of the other tests say t_tracecoreostldd
    // panics the LDDs will not be released
    User::FreeLogicalDevice(_L("TraceCoreOstLdd"));
    User::FreeLogicalDevice(_L("TcLdd"));

    TInt r = KErrNone;

    
    test.Next(_L("Open test LDD"));
    r = User::LoadLogicalDevice(RTraceCoreTest::Name());
    TEST(r==KErrNone || r==KErrAlreadyExists);
    r = TraceTest.Open();
    TEST_KErrNone(r);

    ValidateAllFiltersInSync();
    
    ActivatePrintfHandler(EFalse);
    
    test.Next(_L("Test Basic TraceCore Activation"));
    TestBasicActivation();
    test.Next(_L("Test Basic TraceCore Deactivation"));
    TestBasicDeactivation();
    test.Next(_L("Negative Test TraceCore Activation"));
    NegativeTestActivation();
    test.Next(_L("Negative Test TraceCore Deactivation"));
    NegativeTestDeactivation();
    test.Next(_L("Test Multiple TraceCore Activation"));
    TestMultipleActivation();
    test.Next(_L("Test Multiple TraceCore Deactivation"));
    TestMultipleDeactivation();
    test.Next(_L("Test Big Trace"));
    TestBigTrace();
    test.Next(_L("Test Dropped Traces"));
    TestDroppedTraces();
    test.Next(_L("Test Activation Notification"));
    TestActivationNotification();
    test.Next(_L("Test Refresh Activations"));
    TestRefreshActivations();

    
    ActivatePrintfHandler(ETrue);
    
    ValidateAllFiltersInSync();
    
    test.Next(_L("Close LDD"));
    TraceTest.Close();
    User::FreeLogicalDevice(RTraceCoreTest::Name());


    
    test.Printf(_L("\nFinished Simple Activation/Deactivation tests in trace Core!!!"));
    //test.Getch();
    test.End();
    test.Close();
    return (0);
    }
