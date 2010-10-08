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
#ifndef __TESTDATAWRITERNOTIFIER_H_INCLUDED__
#define __TESTDATAWRITERNOTIFIER_H_INCLUDED__

const int SYMBIAN_TRACE = 0x91;
const int PRINTF_TRACE  = 0x83;
const TInt KTcTdwBufSize = 4096;   

#ifdef _DEBUG
#define TDW_PRINTF(fmt, args...) Kern::Printf(fmt, ## args)
#else
#define TDW_PRINTF(fmt, args...)
#endif


struct TNotifyData
    {
    TNotifyData(TUint32 aAddr, TInt aLen)  
    : iAddr(aAddr)
    , iLen(aLen)
    {    
    }
    
    TInt iAddr;
    TInt iLen;
    
private:
    TNotifyData()
    : iAddr(0)
    , iLen(0)
    {    
    }

};

class MTestWriterNotifier
    {
public:
    virtual void WriteStart() = 0;
    virtual void WriteComplete(TNotifyData aNotifyData) = 0;
    };




#endif /* __TESTDATAWRITERNOTIFIER_H_INCLUDED__ */
