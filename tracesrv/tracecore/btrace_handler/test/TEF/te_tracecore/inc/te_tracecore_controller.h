/**
* Copyright (c) 2002-2009 Nokia Corporation and/or its subsidiary(-ies).
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

#ifndef __TE_TRACECORE_CONTROLLER_H_INCLUDED__
#define __TE_TRACECORE_CONTROLLER_H_INCLUDED__

#include <test/testblockcontroller.h>
#include <test/datadictionary.h>

#include <e32base.h>
#include <e32math.h>

_LIT(KTraceCoreTest, "TraceCoreTest");

class CTraceCoreController : public CTestBlockController
	{
public:
    static CTraceCoreController* NewLC();
    static CTraceCoreController* NewL();
    ~CTraceCoreController();

    virtual CDataWrapper* CreateDataL( const TDesC& aData );

private:
    CTraceCoreController();
	};

#endif // __TE_TRACECORE_CONTROLLER_H_INCLUDED__
