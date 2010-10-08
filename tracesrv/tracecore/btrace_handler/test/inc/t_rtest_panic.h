// Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies).
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

#ifndef __T_RTEST_PANIC_H_INCLUDED__
#define __T_RTEST_PANIC_H_INCLUDED__

#define __E32TEST_EXTENSION__

#include <e32test.h>

#ifdef USE_RTEST_MACROS

#define TEST_FAILED()       { User::Panic(_L("USER"),84); }
#define TEST_EQUAL(e, a)    test_Equal(e, a)
#define TEST_COMPARE(a,b,c) test_Compare(a,b,c)
#define TEST_KErrNone(x)    test_KErrNone(x)
#define TEST(x)             test(x)

#else

static TPtrC GetCurrentSourceFileName(const TText* aFilePath)
    {
    TPtrC filePath(aFilePath);
    TInt pos = filePath.LocateReverse('\\');
    if (pos < 0)
        {
        pos = filePath.LocateReverse('/');
        }
    TPtrC fileName;
    if (pos < 0)
        {
        fileName.Set(filePath);
        }
    else
        {
        fileName.Set(filePath.Right(filePath.Length() - pos - 1));
        }
    return fileName;
    }

#define TEST_FAILED()       { User::Panic(GetCurrentSourceFileName(__S(__FILE__)),__LINE__); }
#define TEST_EQUAL(e, a)    { TInt _e = TInt(e); TInt _a = TInt(a); if (_e != _a) TEST_FAILED(); }
#define TEST_COMPARE(a,b,c) { TInt _a = TInt(a); TInt _c = TInt(c); if (!(_a b _c)) TEST_FAILED(); }
#define TEST_KErrNone(x)    { TInt _r = (x); if (_r !=KErrNone) TEST_FAILED(); }
#define TEST(x)             { if (!(x)) TEST_FAILED(); }

#endif // USE_RTEST_MACROS

#endif // __T_RTEST_PANIC_H_INCLUDED__
