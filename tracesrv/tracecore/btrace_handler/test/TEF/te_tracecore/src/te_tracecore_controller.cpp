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
//

#include "te_tracecore_controller.h"
#include "te_tracecore_datawrapper.h"

CTraceCoreController::CTraceCoreController()
    {  
    }

CTraceCoreController::~CTraceCoreController()
    {  
    }

CTraceCoreController* CTraceCoreController::NewLC()
    {
    CTraceCoreController* self = new (ELeave)CTraceCoreController();
    CleanupStack::PushL(self);
    // No ConstructL()
    return self;
    }

CTraceCoreController* CTraceCoreController::NewL()
    {
    CTraceCoreController* self=CTraceCoreController::NewLC();
    CleanupStack::Pop(self);
    return self;
    }

CDataWrapper*  CTraceCoreController::CreateDataL( const TDesC& aData ) 
    {
    CDataWrapper* dataWrapper = NULL;
    
    if(aData == KTraceCoreTest)
        {
        dataWrapper = CTraceCoreDataWrapper::NewL();
        }
    
    return dataWrapper; 
    }
    







