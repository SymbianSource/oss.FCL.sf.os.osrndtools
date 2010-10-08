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
// e32test\ost\flush_page_cache.cpp
// Overview:
// Tests WDP TraceCore 
// kernel - side APIs
//

#define __E32TEST_EXTENSION__
#include <e32std.h>
#include <dptest.h>
#include "t_rtest_panic.h"

RTest test(_L("FLUSH_PAGE_CACHE"));

TInt ContinueFlushingPageCache()
    {
    RTimer timer;
    TRequestStatus status;
    TInt ret = timer.CreateLocal();
    if (ret!=KErrNone)
        {
        test.Printf(_L("\nUnable to create RTimer object. Error: %d"), ret);
        return ret;
        }
    //repeat flush for 7 minutes
    timer.After(status, 420000000);
    
    while (status==KRequestPending)
        {
        //Flush the cache
        DPTest::FlushCache();
        //wait for 100 microseconds
        User::After(100);
        }
    return KErrNone;
    }



GLDEF_C TInt E32Main()
    {

    test.Title();
    test.Start(_L("Flushing Page Cache"));

    test.Next(_L("Flush every 100us for 7 minutes"));
    TInt err = ContinueFlushingPageCache();
    
    TEST_KErrNone(err);
    
    test.Printf(_L("\nFinished Flushing Page Cache!!!"));

    test.End();
    test.Close();
    return (0);
    }
