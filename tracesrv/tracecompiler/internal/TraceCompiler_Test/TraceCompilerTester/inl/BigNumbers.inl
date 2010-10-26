/*
* Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies).
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

#ifndef __TEST_INL__
#define __TEST_INL__

#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "BigNumbersTraces.h"
#endif

inline TUint32 GetBigValue1000()
    {
    TUint32 value = 1000;
    OstTrace1(INCLUDED_NON_SOURCE_FILE_TEST, NON_SOURCE_FILE_TRACE1, "Inline function GetBigValue1000 return value: %d", value);
    return value;
    }
    
inline TUint32 GetBigValue2000()
    {
    TUint32 value = 2000;
    OstTrace1(INCLUDED_NON_SOURCE_FILE_TEST, NON_SOURCE_FILE_TRACE2, "Inline function GetBigValue1000 return value: %d", value);
    return value;
    }
#endif
