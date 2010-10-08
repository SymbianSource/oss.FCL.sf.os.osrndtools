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
//
//

#ifndef __TRACE_CORE_TEST_WRITER_H_INCLUDED__
#define __TRACE_CORE_TEST_WRITER_H_INCLUDED__


#include <kernel.h>

#include "TestDataWriterNotifier.h"


class DTestDataWriter;

class DTraceCoreTestWriter  : public DBase 
    { 

public:
    static DTraceCoreTestWriter* GetInstance();
    void SetNotifier(MTestWriterNotifier* aNotifier);

    virtual ~DTraceCoreTestWriter();

private:
    TInt Init();    
    
    DTraceCoreTestWriter();
   
private:
    DTestDataWriter* iDataWriter;
    static DTraceCoreTestWriter* iInstance;
};

#endif  // __TRACE_CORE_TEST_WRITER_H_INCLUDED__
