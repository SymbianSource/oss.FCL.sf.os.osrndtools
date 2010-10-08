/**
* Copyright (c) 2004-2009 Nokia Corporation and/or its subsidiary(-ies).
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

#ifndef __TE_TRACECORE_SERVER_H_INCLUDED__
#define __TE_TRACECORE_SERVER_H_INCLUDED__

#include <test/testserver2.h>

class CTe_TraceCoreSuite : public CTestServer2
    {
public:
    static CTe_TraceCoreSuite* NewL();
    virtual CTestBlockController* CreateTestBlock();
    };

#endif // __TE_TRACECORE_SERVER_H_INCLUDED__
