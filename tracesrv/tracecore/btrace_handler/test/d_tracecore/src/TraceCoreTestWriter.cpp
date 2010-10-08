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

#include "TraceCore.h"
#include "TestDataWriter.h"
#include "TraceCoreTestWriter.h"

DTraceCoreTestWriter* DTraceCoreTestWriter::iInstance = NULL;    

DTraceCoreTestWriter* DTraceCoreTestWriter::GetInstance()
    { 
    if ( iInstance == NULL )
        {    
        DTraceCoreTestWriter* instance = new DTraceCoreTestWriter();

        if ( instance != NULL )
            {
            iInstance = instance;            
            TInt ret = instance->Init();
            if ( ret != KErrNone )
                {
                TDW_PRINTF("TraceCoreTestWriter::GetInstance() Init failed ret = %d !!)", ret);
                delete instance;
                iInstance = NULL;
                }
            }
        else
            {
            TDW_PRINTF("DTraceCoreTestWriter New returned null");
            }
        }

    TDW_PRINTF("TraceCoreTestWriter::GetInstance() iInstance = 0x%x)", iInstance);
    
    return iInstance;
    }

void DTraceCoreTestWriter::SetNotifier(MTestWriterNotifier* aNotifier)
    {
    if(iDataWriter)
        {
        iDataWriter->iNotifier = aNotifier;  
        }
    }

DTraceCoreTestWriter::DTraceCoreTestWriter()
: iDataWriter(NULL)
    {
    }

DTraceCoreTestWriter::~DTraceCoreTestWriter()
	{
	  delete iDataWriter;	  
	  iInstance = NULL;
	}


TInt DTraceCoreTestWriter::Init()
    {
    TInt ret = KErrNoMemory;
    if(!iDataWriter)
        {
        iDataWriter = new DTestDataWriter();
        iDataWriter->Init();
        
        // now make sure that we are the writer in use 
        DTraceCore* traceCore = DTraceCore::GetInstance();
        if(traceCore)
            {     
            traceCore->SwitchToWriter(EWriterTypeTest); 
            ret = KErrNone;
            }
        }
    return ret;
    }
