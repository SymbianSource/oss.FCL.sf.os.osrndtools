// Copyright (c) 2005-2009 Nokia Corporation and/or its subsidiary(-ies).
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
// 
//


#include <e32base.h>
#include <e32std.h>

#include "te_tracecore_server.h"
#include "te_tracecore_controller.h"

CTe_TraceCoreSuite* CTe_TraceCoreSuite::NewL()
    {
    CTe_TraceCoreSuite* self=new (ELeave) CTe_TraceCoreSuite;
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
    }

CTestBlockController* CTe_TraceCoreSuite::CreateTestBlock()
    {
    CTestBlockController* controller=NULL;
    TRAP_IGNORE(
            {
            controller = CTraceCoreController::NewL();
            });
    return controller;
    }


LOCAL_C void MainL()
    {
    CActiveScheduler* scheduler = new(ELeave)CActiveScheduler();
    CActiveScheduler::Install(scheduler);
    CTe_TraceCoreSuite* server=NULL;
    TRAPD(err,server = CTe_TraceCoreSuite::NewL());
    if(!err)
        {
        RProcess::Rendezvous(KErrNone);
        scheduler->Start();
        }
    delete server;
    delete scheduler;
    }

GLDEF_C TInt E32Main()
    {
    __UHEAP_MARK;
    CTrapCleanup* cleanup = CTrapCleanup::New();
    if(! cleanup )
        {
        return KErrNoMemory;
        }
    TRAP_IGNORE(MainL());
    delete cleanup;
    __UHEAP_MARKEND;
    return KErrNone;
    }


