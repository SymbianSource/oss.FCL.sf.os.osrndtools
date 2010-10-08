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
// e32test\ost\t_tracecore.h
// Overview:
// Tests activation/deactivation of traces using the TraceCore 
// kernel - side APIs
//

#ifndef T_TRACECORE_H 
#define T_TRACECORE_H

#include <opensystemtrace.h>

const TComponentId KTestComponentID1 = 0x89abcdef;
const TComponentId KTestComponentID2 = KTestComponentID1 - 1;
const TGroupId KTestGroupId1 = TRACE_NORMAL;
const TGroupId KTestGroupId2 = TRACE_IMPORTANT;
const TUint32 KTestData = 0x333;
const TInt KNumTraces=64;

#define KTestPrintfTraceString "Test Printf Trace"
_LIT8(KTestPrintfTraceLiteral, KTestPrintfTraceString);

#define TEST_TRACE 0x860001


#endif //T_TRACECORE_H
